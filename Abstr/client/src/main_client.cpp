#include "../include/AudioStream.hpp"
#include "../include/OpusCodec.hpp"
#include "../include/NetworkClient.hpp"
#include <asio.hpp>
#include <iostream>
#include <atomic>

constexpr int SAMPLE_RATE = 48000;
constexpr int FRAMES_PER_BUFFER = 960;
constexpr int CHANNELS = 1;

class VoiceClient {
public:
    VoiceClient(const std::string& host, unsigned short port)
        : audioStream(SAMPLE_RATE, FRAMES_PER_BUFFER, CHANNELS),
          opusCodec(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP),
          networkClient(ioContext, host, port) {}

    void start() {
        audioStream.startCapture([this](const auto& data) {
            auto encoded = opusCodec.encode(data);
            networkClient.send(encoded);
        });
        
        if(!audioStream.startPlayback()) {
            throw std::runtime_error("Playback initialization failed");
        }

        networkClient.start([this](const auto& data) {
            auto decoded = opusCodec.decode(data);
            audioStream.writePlaybackBuffer(decoded);
        });
    }

private:
    asio::io_context ioContext;
    AudioStream audioStream;
    OpusCodec opusCodec;
    NetworkClient networkClient;
};

int main(int argc, char* argv[]) {
    if(argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>\n";
        return 1;
    }

    try {
        VoiceClient client(argv[1], std::stoi(argv[2]));
        client.start();
        while(true) std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}