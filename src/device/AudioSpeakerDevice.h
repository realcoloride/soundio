#pragma once

#include "./AudioDevice.h"

class AudioSpeakerDevice : public AudioDevice {
protected:
    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) override {
        (void)pDevice;
        (void)pInput;

        // here I'd pull audio from the connected input node into `pOutput`
        if (inputNode) {
            // example: inputNode->fillOutputBuffer(pOutput, frameCount);
        }
    }

public:
    ma_result subscribe(AudioNode* audioNode) override { return subscribeInput(audioNode); }
    ma_result unsubscribe() override { return unsubscribeInput(); }
};
