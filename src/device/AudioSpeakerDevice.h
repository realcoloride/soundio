#pragma once

#include "./AudioDevice.h"
#include "../output/AudioOutput.h"
#include "../core/AudioEndpoint.h"

class AudioSpeakerDevice : public AudioDevice, public virtual AudioOutput {
protected:
    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) override {
        (void)pDevice;

        if (!isAwake || !isInputSubscribed())
            return;

        if (auto ep = dynamic_cast<AudioEndpoint*>(inputNode))
            pullFromEndpoint(ep, pOutput, frameCount); // This drains mic’s outputRing
    }

public:
    AudioSpeakerDevice(const std::string& id, ma_context* context)
        : AudioDevice(id, context)
    {
        canFillInputRing = false;   // we have an input ring for upstream data
        canDrainOutputRing = false; // speakers don't push further downstream
    }
};
