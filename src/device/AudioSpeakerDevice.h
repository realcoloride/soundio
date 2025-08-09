#pragma once

#include "./AudioDevice.h"
#include "../output/AudioDeviceOutput.h"

class AudioSpeakerDevice : public AudioDevice {
protected:
    AudioDeviceOutput* outputSource = nullptr;

    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) override {
        (void)pDevice;
        (void)pInput;

        // fast exit if sleeping or not wired up
        if (!isAwake || !outputSource)
            return;

        // guard against stale pointer
        if (inputNode != outputSource) {
            outputSource = nullptr;
            return;
        }

        // pull PCM from upstream into pOutput
        outputSource->readPCM(pOutput, frameCount);
    }

public:
    ma_result handleInputSubscribe(AudioNode* node) override {
        outputSource = dynamic_cast<AudioDeviceOutput*>(node);
        return MA_SUCCESS;
    }
    ma_result handleInputUnsubscribe(AudioNode* node) override {
        outputSource = nullptr;
        return MA_SUCCESS;
    }

    bool isSubscribed() override { return isInputSubscribed(); }
    ma_result subscribe(AudioDeviceOutput* node) { return subscribeInput(node); }
    ma_result unsubscribe() override { return unsubscribeInput(); }
};
