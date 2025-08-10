#pragma once
#include "./AudioDevice.h"
#include "../input/AudioInput.h"

class AudioMicrophoneDevice : public AudioDevice, public virtual AudioInput {
protected:
    void dataCallback(ma_device* pDevice, void* /*pOutput*/, const void* pInput, ma_uint32 frameCount) override {
        (void)pDevice;

        if (!isAwake || !canFillInputRing || pInput == nullptr)
            return;

        receivePCM(pInput, frameCount);
        mixPCM();
    }

public:
    AudioMicrophoneDevice(const std::string& id, ma_context* context)
        : AudioDevice(id, context)
    {
        canFillInputRing = true;
        canDrainOutputRing = true; // so downstream can pull
    }
};
