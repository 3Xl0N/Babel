#include "client.hpp"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <cstdio>

// Audio parameters
constexpr int SAMPLE_RATE = 48000;
constexpr unsigned long FRAMES_PER_BUFFER = 1024;  // ~21ms at 48kHz
constexpr int CHANNELS = 1;

// -----------------
// Constructor
// -----------------
Client::Client(asio::io_context& io_context, const std::string& server_ip, unsigned short server_port)
    : io_context_(io_context), socket_(io_context), running_(false),
      inputStream_(nullptr), outputStream_(nullptr)
{
    asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(server_ip), server_port);
    socket_.async_connect(endpoint, [this](std::error_code ec) {
        if (!ec) {
            std::cout << "Connected to server.\n";
            do_read();
        } else {
            std::cerr << "Connect error: " << ec.message() << "\n";
        }
    });
}

// -----------------
// Destructor
// -----------------
Client::~Client() {
    stop();
}

// -----------------
// Start: Initialize PortAudio (in full-duplex callback mode) and start threads
// -----------------
void Client::start() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    
    // Setup input stream (callback mode)
    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    if (inputParams.device == paNoDevice) {
        std::cerr << "No default input device.\n";
        return;
    }
    inputParams.channelCount = CHANNELS;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultHighInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;
    
    err = Pa_OpenStream(&inputStream_, &inputParams, nullptr, SAMPLE_RATE,
                        FRAMES_PER_BUFFER, paClipOff, paInputCallback, this);
    if (err != paNoError) {
        std::cerr << "Failed to open input stream: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    
    // Setup output stream (callback mode)
    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
    if (outputParams.device == paNoDevice) {
        std::cerr << "No default output device.\n";
        return;
    }
    outputParams.channelCount = CHANNELS;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;
    
    err = Pa_OpenStream(&outputStream_, nullptr, &outputParams, SAMPLE_RATE,
                        FRAMES_PER_BUFFER, paClipOff, paOutputCallback, this);
    if (err != paNoError) {
        std::cerr << "Failed to open output stream: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    
    err = Pa_StartStream(inputStream_);
    if (err != paNoError) {
        std::cerr << "Failed to start input stream: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    err = Pa_StartStream(outputStream_);
    if (err != paNoError) {
        std::cerr << "Failed to start output stream: " << Pa_GetErrorText(err) << "\n";
        return;
    }
    
    running_ = true;
    // Start network send and receive threads.
    netSendThread_ = std::thread(&Client::network_send_thread, this);
    netReceiveThread_ = std::thread(&Client::network_receive_thread, this);
}

// -----------------
// Stop: Clean up streams, socket, threads
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
}

// -----------------
// PortAudio Input Callback: Captures audio and pushes into captureBuffer.
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
        std::cerr << "Warning: Input overflowed\n";
    
    const float *in = static_cast<const float *>(input);
    // Copy the captured block into a vector.
    std::vector<float> block(in, in + frameCount * CHANNELS);
    
    // Push block into captureBuffer; if full, drop the block.
    if (!client->captureBuffer.push(block))
        std::cerr << "Warning: Capture buffer full, dropping block.\n";
    
    return paContinue;
}

// -----------------
// PortAudio Output Callback: Pulls audio from playbackBuffer for playback.
int Client::paOutputCallback(const void * /*input*/, void *output,
                             unsigned long frameCount,
                             const PaStreamCallbackTimeInfo * /*timeInfo*/,
                             PaStreamCallbackFlags /*statusFlags*/,
                             void *userData)
{
    Client *client = static_cast<Client *>(userData);
    float *out = static_cast<float *>(output);
    std::vector<float> block;
    
    // Try to pop a block from the playback buffer.
    if (client->playbackBuffer.pop(block)) {
        // Ensure we have exactly frameCount samples. If block is shorter, pad with zeros.
        size_t samples_needed = frameCount * CHANNELS;
        if (block.size() < samples_needed) {
            block.resize(samples_needed, 0.0f);
        }
        std::memcpy(out, block.data(), samples_needed * sizeof(float));
    } else {
        // No data available: output silence.
        std::memset(out, 0, frameCount * CHANNELS * sizeof(float));
    }
    
    return paContinue;
}

// -----------------
// Network send thread: Encodes captured audio and sends over network.
void Client::network_send_thread() {
    while (running_) {
        std::vector<float> block;
        if (captureBuffer.pop(block)) {
            // Dummy encoding: reinterpret the float block as bytes.
            auto encoded = encode_audio(block);
            networkQueue.push(encoded);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        
        // Send any pending encoded data over the network.
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
// Network receive thread: Reads from network and pushes decoded audio into playbackBuffer.
void Client::network_receive_thread() {
    while (running_) {
        // For simplicity, use asynchronous read.
        // (In a production app, network receive would be handled in a callback.)
        std::error_code ec;
        size_t len = socket_.read_some(asio::buffer(read_buffer_, max_length), ec);
        if (ec) {
            std::cerr << "Read error: " << ec.message() << "\n";
            stop();
            break;
        }
        if (len > 0) {
            std::vector<char> encoded_data(read_buffer_, read_buffer_ + len);
            // Dummy decoding.
            auto decoded = decode_audio(encoded_data);
            // Push decoded audio into playback buffer.
            if (!playbackBuffer.push(decoded))
                std::cerr << "Warning: Playback buffer full, dropping audio block.\n";
        }
    }
}

// -----------------
// Asynchronous read initiated once (if not using network_receive_thread)
// Here kept for completeness.
void Client::do_read() {
    socket_.async_read_some(asio::buffer(read_buffer_, max_length),
        [this](std::error_code ec, std::size_t length) {
            if (!ec) {
                // Ensure length is a multiple of sizeof(float)
                if (length % sizeof(float) != 0) {
                    std::cerr << "Received incomplete audio packet. Dropping packet.\n";
                    do_read();
                    return;
                }
                std::vector<char> encoded_data(read_buffer_, read_buffer_ + length);
                auto decoded = decode_audio(encoded_data);
                // Push to playback buffer.
                if (!playbackBuffer.push(decoded))
                    std::cerr << "Warning: Playback buffer full, dropping packet.\n";
                do_read();
            } else {
                std::cerr << "Read error: " << ec.message() << "\n";
                stop();
            }
        });
}

// -----------------
// Network write function.
void Client::do_write(const std::vector<char>& data) {
    asio::async_write(socket_, asio::buffer(data.data(), data.size()),
        [this](std::error_code ec, std::size_t /*length*/) {
            if (ec) {
                std::cerr << "Write error: " << ec.message() << "\n";
                if (ec == asio::error::broken_pipe) {
                    std::cerr << "Broken pipe detected. Stopping client.\n";
                    stop();
                }
            }
        });
}

// -----------------
// Dummy encoding: reinterpret float vector as bytes.
std::vector<char> Client::encode_audio(const std::vector<float>& input) {
    const char* raw = reinterpret_cast<const char*>(input.data());
    size_t byte_count = input.size() * sizeof(float);
    return std::vector<char>(raw, raw + byte_count);
}

// -----------------
// Dummy decoding: reinterpret bytes as float vector.
std::vector<float> Client::decode_audio(const std::vector<char>& input) {
    size_t num_floats = input.size() / sizeof(float);
    std::vector<float> output(num_floats);
    std::memcpy(output.data(), input.data(), num_floats * sizeof(float));
    return output;
}

// -----------------
// Simple playback: (not used because output callback handles playback)
// Kept here for testing.
void Client::audio_playback(const std::vector<float>& audioData) {
    PaError err = Pa_WriteStream(outputStream_, audioData.data(), static_cast<unsigned long>(audioData.size() / CHANNELS));
    if (err == paOutputUnderflowed)
        std::cerr << "Warning: Output underflowed\n";
    else if (err != paNoError)
        std::cerr << "Error writing audio stream: " << Pa_GetErrorText(err) << "\n";
}
