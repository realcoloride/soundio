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
    friend class AudioDevice;

protected:
    bool canFillInputRing = false;
    bool canDrainOutputRing = false;

    bool hasInputToSelfConverter = false;
    ma_data_converter inputToSelfConverter{};
    bool hasSelfToOutputConverter = false;
    ma_data_converter selfToOutputConverter{};
    bool areConvertersReady = false;

    ma_pcm_rb inputRing{};
    ma_pcm_rb outputRing{};
    ma_uint32 inputRingFrames = 0;
    ma_uint32 outputRingFrames = 0;

    AudioFormat inputRingFormat;
    AudioFormat outputRingFormat;

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
        if (outputFormat != nullptr) {
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

    ma_result initializeRings(ma_uint32 inputFramesMultiplier = 8, ma_uint32 outputFramesMultiplier = 8) {
        auto* inFmt = getInputFormat();
        auto* outFmt = getOutputFormat();

        inputRingFrames = (inFmt ? inFmt->sampleRate : audioFormat.sampleRate) / inputFramesMultiplier;
        outputRingFrames = (outFmt ? outFmt->sampleRate : audioFormat.sampleRate) / outputFramesMultiplier;

        ma_result result = MA_SUCCESS;

        if (canFillInputRing) {
            inputRingFormat = inFmt ? *inFmt : audioFormat;
            result = ma_pcm_rb_init(
                inputRingFormat.toMaFormat(),
                inputRingFormat.channels,
                inputRingFrames,
                nullptr, nullptr, &inputRing
            );
            if (result != MA_SUCCESS) return result;
        }

        if (canDrainOutputRing) {
            outputRingFormat = outFmt ? *outFmt : audioFormat;
            result = ma_pcm_rb_init(
                outputRingFormat.toMaFormat(),
                outputRingFormat.channels,
                outputRingFrames,
                nullptr, nullptr, &outputRing
            );
            if (result != MA_SUCCESS) return result;
        }

        return MA_SUCCESS;
    }

    // Write to a ring buffer using acquire/commit
    void writeRing(ma_pcm_rb& rb, const AudioFormat& fmt, const void* pData, ma_uint32 frames) {
        ma_uint32 framesToWrite = frames;
        while (framesToWrite > 0) {
            void* pDst = nullptr;
            ma_uint32 writable = framesToWrite;
            if (ma_pcm_rb_acquire_write(&rb, &writable, &pDst) != MA_SUCCESS || writable == 0) break;

            memcpy(pDst, pData, fmt.frameSizeInBytes(writable));
            ma_pcm_rb_commit_write(&rb, writable);

            framesToWrite -= writable;
            pData = (const ma_uint8*)pData + fmt.frameSizeInBytes(writable);
        }
    }

    // Read from a ring buffer using acquire/commit
    ma_uint32 readRing(ma_pcm_rb& rb, const AudioFormat& fmt, void* pOut, ma_uint32 frames) {
        ma_uint32 totalRead = 0;
        ma_uint32 framesToRead = frames;

        while (framesToRead > 0) {
            void* pSrc = nullptr;
            ma_uint32 readable = framesToRead;
            if (ma_pcm_rb_acquire_read(&rb, &readable, &pSrc) != MA_SUCCESS || readable == 0) break;

            memcpy(pOut, pSrc, fmt.frameSizeInBytes(readable));
            ma_pcm_rb_commit_read(&rb, readable);

            totalRead += readable;
            framesToRead -= readable;
            pOut = (ma_uint8*)pOut + fmt.frameSizeInBytes(readable);
        }
        return totalRead;
    }

    // INPUT -> SELF
    void receivePCM(const void* pData, ma_uint32 frameCount) {
        if (!canFillInputRing) return;

        if (hasInputToSelfConverter) {
            std::vector<uint8_t> temp(audioFormat.frameSizeInBytes(frameCount));
            ma_uint64 inF = frameCount, outF = frameCount;
            ma_data_converter_process_pcm_frames(&inputToSelfConverter, pData, &inF, temp.data(), &outF);
            writeRing(inputRing, inputRingFormat, temp.data(), (ma_uint32)outF);
        }
        else {
            writeRing(inputRing, inputRingFormat, pData, frameCount);
        }
        whenInputSubmitted();
    }

    // SELF -> OUTPUT
    ma_uint32 submitPCM(void* pOut, ma_uint32 frameCount) {
        if (!canDrainOutputRing) return 0;
        ma_uint32 read = readRing(outputRing, outputRingFormat, pOut, frameCount);
        whenOutputSubmitted();
        return read;
    }

    // Mix: input ring -> convert to output -> output ring
    ma_result mixPCM() {
        if (!canFillInputRing || !canDrainOutputRing)
            return MA_INVALID_OPERATION;

        ma_uint32 available = ma_pcm_rb_available_read(&inputRing);
        if (available == 0)
            return MA_NO_DATA_AVAILABLE;

        std::vector<uint8_t> temp(audioFormat.frameSizeInBytes(available));
        readRing(inputRing, inputRingFormat, temp.data(), available);

        if (hasSelfToOutputConverter) {
            std::vector<uint8_t> converted(audioFormat.frameSizeInBytes(available));
            ma_uint64 inF = available, outF = available;
            ma_result res = ma_data_converter_process_pcm_frames(&selfToOutputConverter, temp.data(), &inF, converted.data(), &outF);
            if (res != MA_SUCCESS) return res;
            writeRing(outputRing, outputRingFormat, converted.data(), (ma_uint32)outF);
        }
        else {
            writeRing(outputRing, outputRingFormat, temp.data(), available);
        }
        return handleMixPCM(MA_SUCCESS);
    }

    ma_result handleInputSubscribe(AudioNode*) override { renegotiate(); return MA_SUCCESS; }
    ma_result handleOutputSubscribe(AudioNode*) override { renegotiate(); return MA_SUCCESS; }
    ma_result handleInputUnsubscribe(AudioNode*) override { renegotiate(); return MA_SUCCESS; }
    ma_result handleOutputUnsubscribe(AudioNode*) override { renegotiate(); return MA_SUCCESS; }

    virtual ma_result handleMixPCM(ma_result prevResult) { (void)prevResult; return MA_SUCCESS; }
    virtual void whenInputSubmitted() {}
    virtual void whenOutputSubmitted() {}

    void renegotiate() {
        this->isNegociationDone = false;

        // Rebuild converters
        ma_result result = buildConverters();
        if (result != MA_SUCCESS) {
            this->isNegociationDone = false;
            return;
        }

        // Rebuild rings using connected formats
        auto* inFmt = getInputFormat();
        auto* outFmt = getOutputFormat();

        ma_uint32 inputFrames = (inFmt ? inFmt->sampleRate : audioFormat.sampleRate) / 8; // ~8 chunks/sec
        ma_uint32 outputFrames = (outFmt ? outFmt->sampleRate : audioFormat.sampleRate) / 8;

        result = initializeRings(inputFrames, outputFrames);
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
