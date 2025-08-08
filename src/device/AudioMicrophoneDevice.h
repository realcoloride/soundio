#pragma once

#include "./AudioDevice.h"
#include "../input/AudioDeviceInput.h"

class AudioMicrophoneDevice : public AudioDevice {
protected:
    ma_encoder encoder;
    ma_encoder_config encoderConfig;

    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) override {
        (void)pDevice;
        (void)pOutput;

        if (!outputNode || !isAwake) return;
        if (auto* input = getInput())
            input->submitPCM(pInput, frameCount);
    }

    AudioDeviceInput* getInput() {
        return dynamic_cast<AudioDeviceInput*>(outputNode);
    }

public:
    bool isSubscribed() override { return isOutputSubscribed(); }
    ma_result subscribe(AudioDeviceInput* node) { return subscribeOutput(node); }
    ma_result unsubscribe() override { return unsubscribeOutput(); }
};
