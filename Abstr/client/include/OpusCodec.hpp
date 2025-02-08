#ifndef OPUS_CODEC_H
#define OPUS_CODEC_H

#include <opus/opus.h>
#include <vector>

class OpusCodec {
public:
    OpusCodec(int sampleRate, int channels, int application);
    ~OpusCodec();

    std::vector<char> encode(const std::vector<float>& audio);
    std::vector<float> decode(const std::vector<char>& packet);

private:
    OpusEncoder* encoder = nullptr;
    OpusDecoder* decoder = nullptr;
    int frameSize;
};

#endif
