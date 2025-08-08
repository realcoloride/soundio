#pragma once

#include "../include.h"
#include "AudioInput.h"

class AudioFileInput : public AudioInput {
    ma_decoder decoder{};
    AudioFormat mFormat{};

protected:
    ma_result handleOutputSubscribe(AudioNode* dest) override {
        (void)dest;
        return MA_SUCCESS;
    }

    ma_result handleOutputUnsubscribe(AudioNode*) override {
        ma_decoder_uninit(&decoder);
        return MA_SUCCESS;
    }

public:
    AudioFileInput() = default;

    ~AudioFileInput() override {
        ma_decoder_uninit(&decoder);
    }

    ma_result open(const char* filePath) {
        ma_result r = ma_decoder_init_file(filePath, nullptr, &decoder);
        if (r != MA_SUCCESS) return r;

        mFormat = AudioFormat{
            decoder.outputFormat,
            decoder.outputChannels,
            decoder.outputSampleRate
        };
        return MA_SUCCESS;
    }

    ma_data_source* dataSource() override {
        return reinterpret_cast<ma_data_source*>(&decoder);
    }

    AudioFormat format() const override {
        return mFormat;
    }
};
