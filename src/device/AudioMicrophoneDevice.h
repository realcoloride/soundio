#pragma once

#include "./AudioDevice.h"
#include "../input/AudioInput.h"
#include "../output/AudioOutput.h"

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
    AudioMicrophoneDevice(const std::string& id) : AudioDevice(id) {}

    // Implement pure virtuals from AudioNode via AudioEndpoint path
    ma_data_source* dataSource() override { return nullptr; }
    AudioFormat format() const override { return audioFormat; }
};
