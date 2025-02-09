#include "../include/AudioStream.hpp"
#include <iostream>

AudioStream::AudioStream(unsigned sampleRate, unsigned framesPerBuffer, unsigned channels) 
    : sampleRate(sampleRate), framesPerBuffer(framesPerBuffer), channels(channels) {}

AudioStream::~AudioStream() { stop(); }

bool AudioStream::startCapture(Callback callback) {
    captureCallback = callback;
    
    PaError err = Pa_Initialize();
    if(err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    PaStreamParameters inputParams;
    inputParams.device = Pa_GetDefaultInputDevice();
    if(inputParams.device == paNoDevice) {
        std::cerr << "No default input device" << std::endl;
        return false;
    }
    inputParams.channelCount = channels;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultHighInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(&inputStream, &inputParams, nullptr, sampleRate, 
                       framesPerBuffer, paClipOff, paCaptureCallback, this);
    if(err != paNoError) {
        std::cerr << "Failed to open input stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    err = Pa_StartStream(inputStream);
    if(err != paNoError) {
        std::cerr << "Failed to start input stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    return true;
}

bool AudioStream::startPlayback() {
    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
    if(outputParams.device == paNoDevice) {
        std::cerr << "No default output device" << std::endl;
        return false;
    }
    outputParams.channelCount = channels;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(&outputStream, nullptr, &outputParams, sampleRate,
                               framesPerBuffer, paClipOff, paPlaybackCallback, this);
    if(err != paNoError) {
        std::cerr << "Failed to open output stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    err = Pa_StartStream(outputStream);
    if(err != paNoError) {
        std::cerr << "Failed to start output stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    return true;
}

void AudioStream::stop() {
    if(inputStream) {
        Pa_StopStream(inputStream);
        Pa_CloseStream(inputStream);
        inputStream = nullptr;
    }
    if(outputStream) {
        Pa_StopStream(outputStream);
        Pa_CloseStream(outputStream);
        outputStream = nullptr;
    }
    Pa_Terminate();
}

int AudioStream::paCaptureCallback(const void* input, void* /*output*/,
                                  unsigned long frameCount,
                                  const PaStreamCallbackTimeInfo* /*timeInfo*/,
                                  PaStreamCallbackFlags statusFlags,
                                  void* userData) {
    auto* stream = static_cast<AudioStream*>(userData);
    const float* in = static_cast<const float*>(input);
    
    if(statusFlags & paInputOverflow)
        std::cerr << "Input overflow warning" << std::endl;

    std::vector<float> buffer(in, in + frameCount * stream->channels);
    if(stream->captureCallback) {
        stream->captureCallback(buffer);
    }
    stream->captureBuffer.push(buffer);
    
    return paContinue;
}

int AudioStream::paPlaybackCallback(const void* /*input*/, void* output,
                                   unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo* /*timeInfo*/,
                                   PaStreamCallbackFlags /*statusFlags*/,
                                   void* userData) {
    auto* stream = static_cast<AudioStream*>(userData);
    float* out = static_cast<float*>(output);
    std::vector<float> buffer;

    if(stream->playbackBuffer.pop(buffer)) {
        size_t needed = frameCount * stream->channels;
        if(buffer.size() < needed) buffer.resize(needed, 0.0f);
        std::memcpy(out, buffer.data(), needed * sizeof(float));
    } else {
        std::memset(out, 0, frameCount * stream->channels * sizeof(float));
    }
    
    return paContinue;
}

bool AudioStream::readCaptureBuffer(std::vector<float>& data) {
    return captureBuffer.pop(data);
}

bool AudioStream::writePlaybackBuffer(const std::vector<float>& data) {
    return playbackBuffer.push(data);
}
