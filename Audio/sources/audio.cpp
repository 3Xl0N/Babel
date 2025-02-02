#include <iostream>
#include <portaudio.h>
#include <opus/opus.h>
#include <vector>

#define SAMPLE_RATE 48000
#define FRAMES_PER_BUFFER 960  // 20 ms à 48 kHz (taille valide pour Opus)
#define NUM_CHANNELS 1

// Fonction pour encoder les données audio avec Opus
std::vector<uint8_t> encodeOpus(const float* input, int frameSize, int sampleRate, int channels) {
    int error;
    OpusEncoder* encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_AUDIO, &error);
    
    if (error != OPUS_OK || encoder == nullptr) {
        std::cerr << "Erreur création encodeur: " << opus_strerror(error) << std::endl;
        return std::vector<uint8_t>();
    }

    const int maxFrameBytes = 4096;
    uint8_t outputBuffer[maxFrameBytes];
    
    int bytesEncoded = opus_encode_float(encoder, input, frameSize, outputBuffer, maxFrameBytes);
    opus_encoder_destroy(encoder);

    if (bytesEncoded <= 0) {
        std::cerr << "Erreur encodage: " << opus_strerror(bytesEncoded) << std::endl;
        return std::vector<uint8_t>();
    }

    return std::vector<uint8_t>(outputBuffer, outputBuffer + bytesEncoded);
}

// Fonction pour décoder les données audio avec Opus
std::vector<float> decodeOpus(const uint8_t* input, int inputSize, int frameSize, int sampleRate, int channels) {
    int error;
    OpusDecoder* decoder = opus_decoder_create(sampleRate, channels, &error);
    
    if (error != OPUS_OK || decoder == nullptr) {
        std::cerr << "Erreur création décodeur: " << opus_strerror(error) << std::endl;
        return std::vector<float>();
    }

    std::vector<float> outputBuffer(frameSize * channels);
    int samplesDecoded = opus_decode_float(decoder, input, inputSize, outputBuffer.data(), frameSize, 0);
    opus_decoder_destroy(decoder);

    if (samplesDecoded <= 0) {
        std::cerr << "Erreur décodage: " << opus_strerror(samplesDecoded) << std::endl;
        return std::vector<float>();
    }

    return std::vector<float>(outputBuffer.begin(), outputBuffer.begin() + samplesDecoded * channels);
}

// Callback audio
static int audioCallback(const void* inputBuffer, void* outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void* userData) {
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;

    const float* inputData = static_cast<const float*>(inputBuffer);
    float* outputData = static_cast<float*>(outputBuffer);

    // Encodage
    std::vector<uint8_t> encodedData = encodeOpus(inputData, framesPerBuffer, SAMPLE_RATE, NUM_CHANNELS);
    if (encodedData.empty()) {
        std::copy(inputData, inputData + framesPerBuffer, outputData);
        return paContinue;
    }

    // Décodage
    std::vector<float> decodedData = decodeOpus(encodedData.data(), encodedData.size(), framesPerBuffer, SAMPLE_RATE, NUM_CHANNELS);
    if (decodedData.empty()) {
        std::copy(inputData, inputData + framesPerBuffer, outputData);
        return paContinue;
    }

    // Copie du résultat
    std::copy(decodedData.begin(), decodedData.end(), outputData);
    return paContinue;
}

int main() {
    // Initialisation PortAudio
    if (Pa_Initialize() != paNoError) {
        std::cerr << "Échec de l'initialisation PortAudio" << std::endl;
        return 1;
    }

    // Configuration du stream
    PaStream* stream;
    PaStreamParameters inputParams, outputParams;
    
    inputParams.device = Pa_GetDefaultInputDevice();
    inputParams.channelCount = NUM_CHANNELS;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    outputParams.device = Pa_GetDefaultOutputDevice();
    outputParams.channelCount = NUM_CHANNELS;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    // Ouverture du stream
    PaError err = Pa_OpenStream(
        &stream,
        &inputParams,
        &outputParams,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        audioCallback,
        nullptr
    );

    if (err != paNoError) {
        std::cerr << "Erreur ouverture stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }

    // Démarrage du stream
    if (Pa_StartStream(stream) != paNoError) {
        std::cerr << "Erreur démarrage stream" << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Stream actif - Appuyez sur ENTER pour arrêter..." << std::endl;
    std::cin.get();

    // Nettoyage
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
