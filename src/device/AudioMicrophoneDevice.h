#pragma once

#include "./AudioDevice.h"
#include "../core/AudioInput.h"

class AudioMicrophoneDevice : public AudioDevice, public AudioInput {
protected:
    AudioDeviceInput* input = nullptr;

    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) override {
        (void)pDevice;
        (void)pOutput;

        // fast exit if sleeping or not wired up
        if (!isAwake || !input)
            return;

        // even if awake, guard against stale pointer
        if (outputNode != input) {
            input = nullptr;
            return;
        }

        // input->submitPCM(pInput, frameCount);
    }

    ma_result handleOutputSubscribe(AudioNode* node) override {
        input = dynamic_cast<AudioDeviceInput*>(node);
        return MA_SUCCESS;
    }
    ma_result handleOutputUnsubscribe(AudioNode* node) override {
        input = nullptr;
        return MA_SUCCESS;
    }

public:

    bool isSubscribed() override { return isOutputSubscribed(); }
    ma_result subscribe(AudioDeviceInput* node) { return subscribeOutput(node); }
    ma_result unsubscribe() override { return unsubscribeOutput(); }
};
