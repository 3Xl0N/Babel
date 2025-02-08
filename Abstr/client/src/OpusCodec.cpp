#include "../include/OpusCodec.hpp"
#include <stdexcept>

OpusCodec::OpusCodec(int sampleRate, int channels, int application) {
    int err;
    encoder = opus_encoder_create(sampleRate, channels, application, &err);
    if(err != OPUS_OK) throw std::runtime_error("Encoder creation failed");

    decoder = opus_decoder_create(sampleRate, channels, &err);
    if(err != OPUS_OK) throw std::runtime_error("Decoder creation failed");

    frameSize = sampleRate * 20 / 1000; // 20ms frame
}

OpusCodec::~OpusCodec() {
    if(encoder) opus_encoder_destroy(encoder);
    if(decoder) opus_decoder_destroy(decoder);
}

std::vector<char> OpusCodec::encode(const std::vector<float>& audio) {
    std::vector<unsigned char> output(4000);
    int len = opus_encode_float(encoder, audio.data(), frameSize, output.data(), output.size());
    if(len < 0) throw std::runtime_error("Encoding error");
    return std::vector<char>(output.begin(), output.begin() + len);
}

std::vector<float> OpusCodec::decode(const std::vector<char>& packet) {
    std::vector<float> output(frameSize * 2); // Safety margin
    int len = opus_decode_float(decoder, 
                               reinterpret_cast<const unsigned char*>(packet.data()), 
                               packet.size(), 
                               output.data(), 
                               frameSize, 
                               0);
    if(len < 0) throw std::runtime_error("Decoding error");
    output.resize(len * 2); // Stereo
    return output;
}