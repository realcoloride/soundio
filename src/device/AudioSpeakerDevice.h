#pragma once

#include "./AudioDevice.h"
#include "../output/AudioOutput.h"

class AudioSpeakerDevice : public AudioDevice, public virtual AudioOutput {
protected:
    void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) override {
        (void)pDevice;
        (void)pInput;

        // fast exit if sleeping or not wired up
        if (!isAwake || !isInputSubscribed()) {
            // No input node connected -> silence
            if (pOutput) 
                memset(pOutput, 0, frameCount * audioFormat.frameSizeInBytes(1));
            
            return;
        }

        ma_uint32 framesRead = submitPCM(pOutput, frameCount);

        // If underrun, fill remainder with silence
        if (framesRead < frameCount && pOutput) {
            uint8_t* outBytes = reinterpret_cast<uint8_t*>(pOutput);
            size_t offset = framesRead * audioFormat.frameSizeInBytes(1);
            memset(outBytes + offset, 0, (frameCount - framesRead) * audioFormat.frameSizeInBytes(1));
        }
    }

public:
    AudioSpeakerDevice(const std::string& id, ma_context* context): AudioDevice(id, context) {}
};
