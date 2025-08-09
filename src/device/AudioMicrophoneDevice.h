#pragma once

#include "./AudioDevice.h"
#include "../core/AudioInput.h"

class AudioMicrophoneDevice : public AudioDevice, public AudioInput {
protected:
    // is outputnode, but type safetied
    AudioOutput* output = nullptr;

    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) override {
        (void)pDevice;
        (void)pOutput;

        // fast exit if sleeping or not wired up
        if (!isAwake || !isOutputSubscribed())
            return;

        this->submitPCM(pInput, frameCount);
    }

    ma_result handleOutputSubscribe(AudioNode* node) override {
        // Call base if it has important behavior (format negotiation, etc.)
        ma_result result = AudioInput::handleOutputSubscribe(node);
        if (result != MA_SUCCESS)
            return result;

        // outputnode is assigned automatically after this call
        output = dynamic_cast<AudioOutput*>(node);
        
        return MA_SUCCESS;
    }
    ma_result handleOutputUnsubscribe(AudioNode* node) override {
        output = nullptr;
        return AudioInput::handleOutputUnsubscribe(node);
    }

public:

    bool isSubscribed() override { return isOutputSubscribed(); }
    ma_result subscribe(AudioOutput* node) { return subscribeOutput(node); }
    ma_result unsubscribe() override { return unsubscribeOutput(); }
};
