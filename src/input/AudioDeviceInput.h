#pragma once

#include "../include.h"
#include "AudioInput.h"

class AudioDeviceInput : public AudioInput {
    ma_pcm_rb ringBuffer{};

protected:
    ma_result handleInputSubscribe(AudioNode* source) override {
        auto* input = dynamic_cast<AudioInput*>(source);
        if (!input) return MA_INVALID_ARGS;

        auto fmt = input->format();

        // allocate buffer (2 seconds)
        ma_pcm_rb_uninit(&ringBuffer);
        ma_pcm_rb_init(fmt.format, fmt.channels,
            fmt.sampleRate * 2, nullptr, nullptr, &ringBuffer);

        return MA_SUCCESS;
    }

    ma_result handleInputUnsubscribe(AudioNode*) override {
        ma_pcm_rb_uninit(&ringBuffer);
        return MA_SUCCESS;
    }

public:
    void submitPCM(const void* frames, ma_uint64 count) {
        ma_uint64 framesWritten = 0;
        ma_pcm_rb_write(&ringBuffer, frames, count, &framesWritten);
    }

    ma_data_source* dataSource() override {
        return reinterpret_cast<ma_data_source*>(&ringBuffer);
    }

    AudioFormat format() const override {
        auto* src = dynamic_cast<AudioInput*>(inputNode);
        return src ? src->format() : AudioFormat::Stereo48kF32();
    }

    ~AudioDeviceInput() override { ma_pcm_rb_uninit(&ringBuffer); }
};
