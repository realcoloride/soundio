#pragma once

#include "./AudioDevice.h"

class AudioMicrophoneDevice : public AudioDevice {
protected:
    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) override {
        (void)pDevice;
        (void)pOutput;

        // here I'd forward `pInput` data to the connected output node.
        if (outputNode) {
            // example: outputNode->processAudio(pInput, frameCount);
        }
    }

public:
    ma_result subscribe(AudioNode* audioNode) override { return subscribeOutput(audioNode); }
    ma_result unsubscribe() override { return unsubscribeOutput(); }
};
