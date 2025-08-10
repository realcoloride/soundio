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
        ma_uint64 framesRemaining = count;
        const uint8_t* pSrc = static_cast<const uint8_t*>(frames);

        while (framesRemaining > 0) {
            void* pDst;
            ma_uint32 framesAvailable;

            ma_pcm_rb_acquire_write(&ringBuffer, &framesAvailable, &pDst);
            if (framesAvailable == 0)
                break;

            ma_uint32 framesToWrite = (ma_uint32)std::min<ma_uint64>(framesRemaining, framesAvailable);
            
            const ma_uint32 frameSize = mFormat.frameSizeInBytes();
            memcpy(pDst, pSrc, framesToWrite * frameSize);
            ma_pcm_rb_commit_write(&ringBuffer, framesToWrite);

            framesRemaining -= framesToWrite;
            pSrc += framesToWrite * frameSize;
        }
    }

    ma_data_source* dataSource() override {
        return reinterpret_cast<ma_data_source*>(&ringBuffer);
    }

    AudioFormat format() const override {
        return mFormat;
    }
};
