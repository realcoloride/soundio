#pragma once

#include "./AudioNode.h"
#include "./AudioFormat.h"

class AudioEndpoint : public AudioNode {
protected:
    ma_data_converter converter{};
    bool              hasConverter = false;

    std::vector<uint8_t> scratchBuffer; // resized only during negotiation (control thread)
    static constexpr ma_uint32 CHUNK_FRAMES = 1024;

    const AudioFormat& producerFormat() const {
        if (auto* up = dynamic_cast<AudioEndpoint*>(inputNode)) return up->audioFormat;
        return audioFormat; // source node: use self format
    }
    const AudioFormat& consumerFormat() const {
        if (auto* down = dynamic_cast<AudioEndpoint*>(outputNode)) return down->audioFormat;
        return audioFormat; // fallback (unused when no output)
    }

    ma_result buildConverter(const AudioFormat& inFmt, const AudioFormat& outFmt) {
        // Control thread only (graph stopped). No allocations in audio thread.
        if (inFmt == outFmt) {
            if (hasConverter) {
                ma_data_converter_uninit(&converter, nullptr);
                hasConverter = false;
            }
            scratchBuffer.clear();
            return MA_SUCCESS;
        }

        if (hasConverter) {
            ma_data_converter_uninit(&converter, nullptr);
            hasConverter = false;
        }

        ma_data_converter_config config = ma_data_converter_config_init(
            inFmt.toMaFormat(),            // formatIn
            outFmt.toMaFormat(),           // formatOut
            inFmt.channels,                // channelsIn
            outFmt.channels,               // channelsOut
            inFmt.sampleRate,               // sampleRateIn
            outFmt.sampleRate                // sampleRateOut
        );
        ma_result r = ma_data_converter_init(&config, nullptr, &converter);
        if (r != MA_SUCCESS) return r;

        hasConverter = true;
        scratchBuffer.resize(static_cast<size_t>(CHUNK_FRAMES) * outFmt.frameSizeBytes());
        return MA_SUCCESS;
    }

    void tryNegotiate() {
        if (!isOutputSubscribed()) return;
        const AudioFormat& inFmt = producerFormat();
        const AudioFormat& outFmt = consumerFormat();
        (void)buildConverter(inFmt, outFmt);
    }

    void forwardToOutput(const void* frames, ma_uint32 frameCount) {
        if (!outputNode) return;
        if (auto* down = dynamic_cast<AudioEndpoint*>(outputNode)) {
            down->receivePCM(frames, frameCount);
        }
    }

public:
    // Call this on control thread after changing this node’s native format.
    virtual void renegotiate() { tryNegotiate(); }

    // AUDIO THREAD: no allocations, no locks.
    void submitPCM(const void* inFrames, ma_uint32 inFrameCount) {
        if (!isOutputSubscribed() || inFrameCount == 0) return;

        if (!hasConverter) {
            forwardToOutput(inFrames, inFrameCount);
            return;
        }

        // Convert in chunks using preallocated scratch.
        const AudioFormat& inFmt = producerFormat();
        (void)inFmt; // For clarity; converter already encodes formats.

        const uint8_t* src = static_cast<const uint8_t*>(inFrames);
        ma_uint64 inLeft = inFrameCount;

        while (inLeft > 0) {
            ma_uint64 inToProcess = std::min<ma_uint64>(inLeft, CHUNK_FRAMES);
            ma_uint64 outProduced = CHUNK_FRAMES;
            ma_result r = ma_data_converter_process_pcm_frames(
                &converter,
                scratchBuffer.data(), &outProduced,
                (void**)&src, &inToProcess
            );
            if (outProduced > 0) {
                forwardToOutput(scratchBuffer.data(), static_cast<ma_uint32>(outProduced));
            }
            if (r != MA_SUCCESS) break;
            inLeft -= inToProcess;
        }
    }

    // Default mid-node behavior: just pass through.
    // Sinks (speaker/file) override this to actually consume.
    virtual void receivePCM(const void* frames, ma_uint32 frameCount) {
        forwardToOutput(frames, frameCount);
    }

protected:
    // Link hooks (CONTROL THREAD ONLY).
    ma_result handleInputSubscribe(AudioNode* src) override {
        ma_result r = AudioNode::handleInputSubscribe(src);
        if (r == MA_SUCCESS) tryNegotiate();
        return r;
    }
    ma_result handleOutputSubscribe(AudioNode* dst) override {
        ma_result r = AudioNode::handleOutputSubscribe(dst);
        if (r == MA_SUCCESS) tryNegotiate();
        return r;
    }
    ma_result handleInputUnsubscribe(AudioNode* src) override {
        if (hasConverter) {
            ma_data_converter_uninit(&converter, nullptr);
            hasConverter = false;
            scratchBuffer.clear();
        }
        return AudioNode::handleInputUnsubscribe(src);
    }
    ma_result handleOutputUnsubscribe(AudioNode* dst) override {
        if (hasConverter) {
            ma_data_converter_uninit(&converter, nullptr);
            hasConverter = false;
            scratchBuffer.clear();
        }
        return AudioNode::handleOutputUnsubscribe(dst);
    }

public:
    virtual ~AudioEndpoint() {
        if (hasConverter) {
            ma_data_converter_uninit(&converter, nullptr);
            hasConverter = false;
        }
    }
};
