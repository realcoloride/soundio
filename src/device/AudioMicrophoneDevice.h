    #pragma once

    #include "./AudioDevice.h"
    #include "../input/AudioInput.h"
    #include "../output/AudioOutput.h"

    class AudioMicrophoneDevice : public AudioDevice, public virtual AudioInput {
    protected:
        void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) override {
            (void)pDevice;
            (void)pOutput;

            // fast exit if sleeping or not wired up
            if (!isAwake || !isOutputSubscribed())
                return;
        
            if (pInput == nullptr) {
                SI_LOG("Mic dataCallback: pInput=null, frames=" << frameCount);
                return;
            }
        
            SI_LOG("Mic dataCallback: frames=" << frameCount);
            this->submitPCM(pInput, frameCount);
        }

    public:
        AudioMicrophoneDevice(const std::string& id, ma_context* context) : AudioDevice(id, context) {}

        // Implement pure virtuals from AudioNode via AudioEndpoint path
        ma_data_source* dataSource() override { return nullptr; }
        AudioFormat format() const override { return audioFormat; }
    };
