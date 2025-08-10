#pragma once

#include "./AudioNode.h"
#include "./AudioFormat.h"

// INPUT  node v
// MIX    self v (SUBMIT DEFINED BY NODE ITSELF)
// OUTPUT node <
//
// AudioEndpoint:
// - Always stores PCM in self format inside its FIFOs.
// - Input FIFO: holds data from upstream (converted to self format).
// - Output FIFO: holds data ready to be consumed by downstream.
// - Converters rebuilt on renegotiation.
// - mixPCM() is fixed pipeline; handleMixPCM() is the hook.

class AudioEndpoint : public virtual AudioNode {
protected:
    // These must be enabled by derived classes
    bool canFillInputRing = false;
    bool canDrainOutputRing = false;

    bool hasInputToSelfConverter = false;
    ma_data_converter inputToSelfConverter{};
    bool hasSelfToOutputConverter = false;
    ma_data_converter selfToOutputConverter{};
    bool areConvertersReady = false;

    // ring buffers
    ma_pcm_rb inputRing{};
    ma_pcm_rb outputRing{};
    ma_uint32 inputRingFrames = 0;
    ma_uint32 outputRingFrames = 0;

    bool isNegociationDone = false;

    AudioFormat* getInputFormat() {
        return !inputNode ? nullptr : &inputNode->audioFormat;
    }
    AudioFormat* getOutputFormat() {
        return !outputNode ? nullptr : &outputNode->audioFormat;
    }

    ma_result buildConverters() {
        this->areConvertersReady = false;

        if (hasInputToSelfConverter) {
            ma_data_converter_uninit(&inputToSelfConverter, nullptr);
            hasInputToSelfConverter = false;
        }
        if (hasSelfToOutputConverter) {
            ma_data_converter_uninit(&selfToOutputConverter, nullptr);
            hasSelfToOutputConverter = false;
        }

        auto* inputFormat = getInputFormat();
        auto* outputFormat = getOutputFormat();

        ma_result result = MA_SUCCESS;

        // I -> SELF
        if (inputFormat != nullptr && audioFormat != *inputFormat) {
            ma_data_converter_config config = ma_data_converter_config_init(
                inputFormat->toMaFormat(),
                audioFormat.toMaFormat(),
                inputFormat->channels,
                audioFormat.channels,
                inputFormat->sampleRate,
                audioFormat.sampleRate
            );
            result = ma_data_converter_init(&config, nullptr, &inputToSelfConverter);
            hasInputToSelfConverter = result == MA_SUCCESS;
        }
        if (result != MA_SUCCESS) return result;

        // SELF -> O
        if (outputFormat != nullptr && audioFormat != *outputFormat) {
            ma_data_converter_config config = ma_data_converter_config_init(
                audioFormat.toMaFormat(),
                outputFormat->toMaFormat(),
                audioFormat.channels,
                outputFormat->channels,
                audioFormat.sampleRate,
                outputFormat->sampleRate
            );
            result = ma_data_converter_init(&config, nullptr, &selfToOutputConverter);
            hasSelfToOutputConverter = result == MA_SUCCESS;
        }
        if (result != MA_SUCCESS) return result;

        this->areConvertersReady = hasInputToSelfConverter || hasSelfToOutputConverter;
        return result;
    }

    ma_result initializeRings(ma_uint32 inputFrames, ma_uint32 outputFrames) {
        inputRingFrames = inputFrames;
        outputRingFrames = outputFrames;

        ma_result result = MA_SUCCESS;

        if (canFillInputRing) {
            result = ma_pcm_rb_init(audioFormat.channels,
                ma_get_bytes_per_sample(audioFormat.toMaFormat()),
                inputFrames, nullptr, nullptr, &inputRing);
            if (result != MA_SUCCESS) return result;
        }

        if (canDrainOutputRing) {
            result = ma_pcm_rb_init(audioFormat.channels,
                ma_get_bytes_per_sample(audioFormat.toMaFormat()),
                outputFrames, nullptr, nullptr, &outputRing);
            if (result != MA_SUCCESS) return result;
        }

        return MA_SUCCESS;
    }

    // INPUT -> SELF (push into input ring)
    void receivePCM(const void* pData, ma_uint32 frameCount) {
        if (!canFillInputRing) return;

        if (hasInputToSelfConverter) {
            std::vector<uint8_t> temp(audioFormat.frameSizeInBytes(frameCount));
            ma_uint64 inFrames = frameCount;
            ma_uint64 outFrames = frameCount;
            ma_data_converter_process_pcm_frames(&inputToSelfConverter, pData, &inFrames, temp.data(), &outFrames);
            ma_pcm_rb_write(&inputRing, temp.data(), (ma_uint32)outFrames);
        }
        else {
            ma_pcm_rb_write(&inputRing, pData, frameCount);
        }

        whenInputSubmitted();
    }

    // SELF -> OUTPUT (pull from output ring)
    ma_uint32 submitPCM(void* pOut, ma_uint32 frameCount) {
        if (!canDrainOutputRing) return 0;
        ma_uint32 framesRead = ma_pcm_rb_read(&outputRing, pOut, frameCount);
        whenOutputSubmitted();
        return framesRead;
    }

    // Fixed mix pipeline: input ring -> conversion -> output ring
    ma_result mixPCM() {
        if (!canFillInputRing || !canDrainOutputRing)
            return MA_INVALID_OPERATION;

        ma_uint32 available = ma_pcm_rb_available_read(&inputRing);
        if (available == 0)
            return MA_NO_DATA_AVAILABLE;

        std::vector<uint8_t> temp(audioFormat.frameSizeInBytes(available));
        ma_pcm_rb_read(&inputRing, temp.data(), available);

        if (hasSelfToOutputConverter) {
            std::vector<uint8_t> converted(audioFormat.frameSizeInBytes(available)); // adjust if conversion changes frames
            ma_uint64 inFrames = available;
            ma_uint64 outFrames = available;
            ma_result res = ma_data_converter_process_pcm_frames(&selfToOutputConverter, temp.data(), &inFrames, converted.data(), &outFrames);
            if (res != MA_SUCCESS) return res;
            ma_pcm_rb_write(&outputRing, converted.data(), (ma_uint32)outFrames);
        }
        else {
            ma_pcm_rb_write(&outputRing, temp.data(), available);
        }

        return handleMixPCM(MA_SUCCESS);
    }

    // Overridable hook for custom processing
    virtual ma_result handleMixPCM(ma_result prevResult) {
        (void)prevResult;
        return MA_SUCCESS;
    }

    // Hooks for node-specific events
    virtual void whenInputSubmitted() {}
    virtual void whenOutputSubmitted() {}

    void renegotiate() {
        this->isNegociationDone = false;
        ma_result result = buildConverters();
        this->isNegociationDone = result == MA_SUCCESS;
    }

public:
    virtual ~AudioEndpoint() {
        if (hasInputToSelfConverter) ma_data_converter_uninit(&inputToSelfConverter, nullptr);
        if (hasSelfToOutputConverter) ma_data_converter_uninit(&selfToOutputConverter, nullptr);
        ma_pcm_rb_uninit(&inputRing);
        ma_pcm_rb_uninit(&outputRing);
    }
};
