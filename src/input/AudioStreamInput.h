#pragma once

#include "../include.h"
#include "AudioInput.h"

class AudioStreamInput : public AudioInput {
    ma_pcm_rb ringBuffer{};
    AudioFormat mFormat;

protected:
    ma_result handleOutputSubscribe(AudioNode* dest) override {
        (void)dest;
        // pre-init with 2 seconds buffer
        ma_pcm_rb_init(mFormat.format, mFormat.channels,
            mFormat.sampleRate * 2, nullptr, nullptr, &ringBuffer);
        return MA_SUCCESS;
    }

    ma_result handleOutputUnsubscribe(AudioNode*) override {
        ma_pcm_rb_uninit(&ringBuffer);
        return MA_SUCCESS;
    }

public:
    AudioStreamInput(AudioFormat fmt = AudioFormat::Stereo48kF32())
        : mFormat(fmt) {
    }

    ~AudioStreamInput() override {
        ma_pcm_rb_uninit(&ringBuffer);
    }

    void pushPCM(const void* frames, ma_uint64 count) {
        ma_uint64 written = 0;
        ma_pcm_rb_write(&ringBuffer, frames, count, &written);
    }

    ma_data_source* dataSource() override {
        return reinterpret_cast<ma_data_source*>(&ringBuffer);
    }

    AudioFormat format() const override {
        return mFormat;
    }
};
