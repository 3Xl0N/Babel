#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <asio.hpp>
#include <portaudio.h>
#include <mutex>
#include <queue>
#include <cstdlib>
#include <atomic>

// --- Simple lock-free ring buffer template ---
template<typename T, size_t Size>
class RingBuffer {
public:
    RingBuffer() : head(0), tail(0) {}
    bool push(const T &item) {
        size_t current_head = head.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) % Size;
        if (next_head == tail.load(std::memory_order_acquire))
            return false; // Buffer full
        buffer[current_head] = item;
        head.store(next_head, std::memory_order_release);
        return true;
    }
    bool pop(T &item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);
        if (current_tail == head.load(std::memory_order_acquire))
            return false; // Buffer empty
        item = buffer[current_tail];
        tail.store((current_tail + 1) % Size, std::memory_order_release);
        return true;
    }
private:
    T buffer[Size];
    std::atomic<size_t> head, tail;
};

class Client {
public:
    Client(asio::io_context& io_context, const std::string& server_ip, unsigned short server_port);
    ~Client();
    
    void start();
    void stop();
    
    // Network I/O
    void do_read();
    void do_write(const std::vector<char>& data);
    
    // Dummy encoding/decoding (replace with Opus routines)
    std::vector<char> encode_audio(const std::vector<float>& input);
    std::vector<float> decode_audio(const std::vector<char>& input);
    
    // Audio playback function (invoked by network receive processing)
    void audio_playback(const std::vector<float>& audioData);
    
    // PortAudio callbacks for input and output
    static int paInputCallback(const void *input, void * /*output*/,
                               unsigned long frameCount,
                               const PaStreamCallbackTimeInfo *timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData);
    static int paOutputCallback(const void * /*input*/, void *output,
                                unsigned long frameCount,
                                const PaStreamCallbackTimeInfo *timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void *userData);
    
    // Processing threads: one for network send (using captured data) and one for network receive.
    void network_send_thread();
    void network_receive_thread();  // For simplicity, assume do_read() posts decoded data to playbackBuffer.
    
    // Ring buffers for decoupling audio processing:
    RingBuffer<std::vector<float>, 256> captureBuffer;   // Captured raw audio blocks
    RingBuffer<std::vector<float>, 256> playbackBuffer;  // Audio blocks to be played back
    RingBuffer<std::vector<char>, 256> networkQueue;     // Encoded data ready to be sent
    
    // ASIO & PortAudio objects.
    asio::io_context& io_context_;
    asio::ip::tcp::socket socket_;
    PaStream* inputStream_;
    PaStream* outputStream_;
    
    // Threads.
    std::thread netSendThread_;
    std::thread netReceiveThread_;
    
    // Atomic flag.
    std::atomic<bool> running_;
    
    enum { max_length = 4096 };
    char read_buffer_[max_length];
};

#endif // CLIENT_HPP