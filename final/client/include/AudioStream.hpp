#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include <portaudio.h>
#include <vector>
#include <functional>
#include "RingBuffer.hpp"

class AudioStream {
public:
    using Callback = std::function<void(const std::vector<float>&)>;

    AudioStream(unsigned sampleRate, unsigned framesPerBuffer, unsigned channels);
    ~AudioStream();

    bool startCapture(Callback callback);
    bool startPlayback();
    void stop();

    bool readCaptureBuffer(std::vector<float>& data);
    bool writePlaybackBuffer(const std::vector<float>& data);

private:
    static int paCaptureCallback(
        const void* input, void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);

    static int paPlaybackCallback(
        const void* input, void* output,
        unsigned long frameCount,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);

    PaStream* inputStream = nullptr;
    PaStream* outputStream = nullptr;
    unsigned sampleRate;
    unsigned framesPerBuffer;
    unsigned channels;
    Callback captureCallback;
    RingBuffer<std::vector<float>, 256> captureBuffer;
    RingBuffer<std::vector<float>, 256> playbackBuffer;
};

#endif
