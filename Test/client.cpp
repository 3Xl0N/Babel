#include "client.hpp"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <cstdio>
#include <vector>
#include <asio.hpp>
#include <portaudio.h>
#include <opus/opus.h>

// Paramètres audio
constexpr int SAMPLE_RATE = 48000;
// Utiliser une taille de trame valide pour Opus : 960 (20 ms à 48 kHz) est couramment utilisée.
constexpr unsigned long FRAMES_PER_BUFFER = 960;
constexpr int CHANNELS = 1;

// --- Variables globales pour Opus ---
OpusEncoder* opus_encoder = nullptr;
OpusDecoder* opus_decoder = nullptr;

// --- Fonction d'initialisation d'Opus ---
bool init_opus() {
    int error;
    opus_encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &error);
    if (error != OPUS_OK) {
        std::cerr << "Erreur lors de la création de l'encodeur Opus: " << opus_strerror(error) << std::endl;
        return false;
    }
    opus_decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
    if (error != OPUS_OK) {
        std::cerr << "Erreur lors de la création du décodeur Opus: " << opus_strerror(error) << std::endl;
        return false;
    }
    return true;
}

// -----------------
// Constructeur
// -----------------
Client::Client(asio::io_context& io_context, const std::string& server_ip, unsigned short server_port)
    : io_context_(io_context), socket_(io_context), running_(false),
      inputStream_(nullptr), outputStream_(nullptr)
{
    asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(server_ip), server_port);
    socket_.async_connect(endpoint, [this](std::error_code ec) {
        if (!ec) {
            std::cout << "Connecté au serveur.\n";
            do_read();
        } else {
            std::cerr << "Erreur de connexion: " << ec.message() << "\n";
        }
    });
}

// -----------------
// Destructeur
// -----------------
Client::~Client() {
    stop();
}

// -----------------
// Méthode start : Initialise PortAudio, Opus et démarre les threads réseau
// -----------------
void Client::start() {
    if (!init_opus()) {
        std::cerr << "Initialisation d'Opus échouée.\n";
        return;
    }
    
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "Erreur PortAudio: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    
    // Configuration du flux d'entrée (capture)
    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    if (inputParams.device == paNoDevice) {
        std::cerr << "Aucun périphérique d'entrée par défaut.\n";
        return;
    }
    inputParams.channelCount = CHANNELS;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultHighInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;
    
    err = Pa_OpenStream(&inputStream_, &inputParams, nullptr, SAMPLE_RATE,
                        FRAMES_PER_BUFFER, paClipOff, paInputCallback, this);
    if (err != paNoError) {
        std::cerr << "Échec de l'ouverture du flux d'entrée: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    
    // Configuration du flux de sortie (lecture)
    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
    if (outputParams.device == paNoDevice) {
        std::cerr << "Aucun périphérique de sortie par défaut.\n";
        return;
    }
    outputParams.channelCount = CHANNELS;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;
    
    err = Pa_OpenStream(&outputStream_, nullptr, &outputParams, SAMPLE_RATE,
                        FRAMES_PER_BUFFER, paClipOff, paOutputCallback, this);
    if (err != paNoError) {
        std::cerr << "Échec de l'ouverture du flux de sortie: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    
    err = Pa_StartStream(inputStream_);
    if (err != paNoError) {
        std::cerr << "Échec du démarrage du flux d'entrée: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    err = Pa_StartStream(outputStream_);
    if (err != paNoError) {
        std::cerr << "Échec du démarrage du flux de sortie: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    
    running_ = true;
    // Démarrer les threads d'envoi et de réception réseau.
    netSendThread_ = std::thread(&Client::network_send_thread, this);
    netReceiveThread_ = std::thread(&Client::network_receive_thread, this);
}

// -----------------
// Méthode stop : Nettoie les streams, le socket, les threads et libère les ressources Opus
// -----------------
void Client::stop() {
    running_ = false;
    if (netSendThread_.joinable())
        netSendThread_.join();
    if (netReceiveThread_.joinable())
        netReceiveThread_.join();
    if (socket_.is_open()) {
        std::error_code ec;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        socket_.close(ec);
    }
    if (inputStream_) {
        Pa_StopStream(inputStream_);
        Pa_CloseStream(inputStream_);
        inputStream_ = nullptr;
    }
    if (outputStream_) {
        Pa_StopStream(outputStream_);
        Pa_CloseStream(outputStream_);
        outputStream_ = nullptr;
    }
    Pa_Terminate();
    
    if (opus_encoder) {
        opus_encoder_destroy(opus_encoder);
        opus_encoder = nullptr;
    }
    if (opus_decoder) {
        opus_decoder_destroy(opus_decoder);
        opus_decoder = nullptr;
    }
}

// -----------------
// Callback d'entrée PortAudio : capture l'audio et pousse dans captureBuffer.
int Client::paInputCallback(const void *input, void * /*output*/,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo * /*timeInfo*/,
                            PaStreamCallbackFlags statusFlags,
                            void *userData)
{
    Client *client = static_cast<Client *>(userData);
    if (!input)
        return paContinue;
    
    if (statusFlags & paInputOverflow)
        std::cerr << "Warning: Débordement d'entrée.\n";
    
    const float *in = static_cast<const float *>(input);
    // Créer un vecteur avec les données capturées (taille = frameCount * CHANNELS)
    std::vector<float> block(in, in + frameCount * CHANNELS);
    
    if (!client->captureBuffer.push(block))
        std::cerr << "Warning: Capture buffer plein, bloc abandonné.\n";
    
    return paContinue;
}

// -----------------
// Callback de sortie PortAudio : récupère l'audio depuis playbackBuffer pour lecture.
int Client::paOutputCallback(const void * /*input*/, void *output,
                             unsigned long frameCount,
                             const PaStreamCallbackTimeInfo * /*timeInfo*/,
                             PaStreamCallbackFlags /*statusFlags*/,
                             void *userData)
{
    Client *client = static_cast<Client *>(userData);
    float *out = static_cast<float *>(output);
    std::vector<float> block;
    
    if (client->playbackBuffer.pop(block)) {
        size_t samples_needed = frameCount * CHANNELS;
        if (block.size() < samples_needed)
            block.resize(samples_needed, 0.0f);
        std::memcpy(out, block.data(), samples_needed * sizeof(float));
    } else {
        std::memset(out, 0, frameCount * CHANNELS * sizeof(float));
    }
    
    return paContinue;
}

// -----------------
// Thread d'envoi réseau : encode l'audio capturé et envoie sur le réseau.
void Client::network_send_thread() {
    while (running_) {
        std::vector<float> block;
        if (captureBuffer.pop(block)) {
            auto encoded = encode_audio(block);
            networkQueue.push(encoded);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        std::vector<char> data;
        if (networkQueue.pop(data)) {
            io_context_.post([this, data]() {
                do_write(data);
            });
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

// -----------------
// Thread de réception réseau : lit les données, décode et pousse dans playbackBuffer.
void Client::network_receive_thread() {
    while (running_) {
        std::error_code ec;
        size_t len = socket_.read_some(asio::buffer(read_buffer_, max_length), ec);
        if (ec) {
            std::cerr << "Erreur de lecture: " << ec.message() << "\n";
            stop();
            break;
        }
        if (len > 0) {
            std::vector<char> encoded_data(read_buffer_, read_buffer_ + len);
            auto decoded = decode_audio(encoded_data);
            if (!playbackBuffer.push(decoded))
                std::cerr << "Warning: Playback buffer plein, bloc abandonné.\n";
        }
    }
}

// -----------------
// Lecture asynchrone (si nécessaire)
void Client::do_read() {
    socket_.async_read_some(asio::buffer(read_buffer_, max_length),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                if (length % sizeof(float) != 0) {
                    std::cerr << "Paquet audio incomplet reçu. Abandon du paquet.\n";
                    do_read();
                    return;
                }
                std::vector<char> encoded_data(read_buffer_, read_buffer_ + length);
                auto decoded = decode_audio(encoded_data);
                if (!playbackBuffer.push(decoded))
                    std::cerr << "Warning: Playback buffer plein, paquet abandonné.\n";
                do_read();
            } else {
                std::cerr << "Erreur de lecture: " << ec.message() << "\n";
                stop();
            }
        });
}

// -----------------
// Fonction d'écriture réseau : envoie des données sur le socket.
void Client::do_write(const std::vector<char>& data) {
    asio::async_write(socket_, asio::buffer(data.data(), data.size()),
        [this](std::error_code ec, std::size_t /*length*/) {
            if (ec) {
                std::cerr << "Erreur d'écriture: " << ec.message() << "\n";
                if (ec == asio::error::broken_pipe) {
                    std::cerr << "Broken pipe détecté. Arrêt du client.\n";
                    stop();
                }
            }
        });
}

// -----------------
// Encodage audio avec Opus
std::vector<char> Client::encode_audio(const std::vector<float>& input) {
    const int max_data_bytes = 4000;
    std::vector<unsigned char> output(max_data_bytes);
    
    // Le frame_size doit correspondre au nombre d'échantillons par canal (ici FRAMES_PER_BUFFER)
    int nb_bytes = opus_encode_float(opus_encoder, input.data(), FRAMES_PER_BUFFER, output.data(), max_data_bytes);
    if (nb_bytes < 0) {
        std::cerr << "Erreur lors de l'encodage Opus: " << opus_strerror(nb_bytes) << "\n";
        return {};
    }
    return std::vector<char>(reinterpret_cast<char*>(output.data()),
                             reinterpret_cast<char*>(output.data()) + nb_bytes);
}

// -----------------
// Décodage audio avec Opus
std::vector<float> Client::decode_audio(const std::vector<char>& input) {
    std::vector<float> output(FRAMES_PER_BUFFER * CHANNELS);
    int nb_samples = opus_decode_float(opus_decoder,
                                         reinterpret_cast<const unsigned char*>(input.data()),
                                         input.size(),
                                         output.data(),
                                         FRAMES_PER_BUFFER,
                                         0);
    if (nb_samples < 0) {
        std::cerr << "Erreur lors du décodage Opus: " << opus_strerror(nb_samples) << "\n";
        return {};
    }
    output.resize(nb_samples * CHANNELS);
    return output;
}

// -----------------
// Fonction de lecture audio (playback) via PortAudio (si nécessaire)
// Ici, cette fonction est utilisée par le callback de sortie.
void Client::audio_playback(const std::vector<float>& audioData) {
    PaError err = Pa_WriteStream(outputStream_, audioData.data(), static_cast<unsigned long>(audioData.size() / CHANNELS));
    if (err == paOutputUnderflowed)
        std::cerr << "Warning: Output underflowed\n";
    else if (err != paNoError)
        std::cerr << "Erreur lors de l'écriture du flux audio: " << Pa_GetErrorText(err) << "\n";
}