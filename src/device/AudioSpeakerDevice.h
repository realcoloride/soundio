#pragma once
#include "./AudioDevice.h"
#include "../output/AudioOutput.h"

class AudioSpeakerDevice : public AudioDevice, public virtual AudioOutput {
protected:
    ma_pcm_rb ring{};
    bool ringInit = false;

    void dataCallback(ma_device*, void* out, const void*, ma_uint32 frames) override {
        if (!isAwake || frames == 0) {
            if (out) ma_silence_pcm_frames(out, frames, audioFormat.toMaFormat(), audioFormat.channels);
            return;
        }

        uint8_t* dst = static_cast<uint8_t*>(out);
        const size_t frameSize = audioFormat.frameSizeBytes();
        ma_uint32 remaining = frames;

        ma_uint32 totalRead = 0;
        while (remaining > 0) {
            void* readPtr = nullptr;
            ma_uint32 avail = 0;
            if (ma_pcm_rb_acquire_read(&ring, &avail, &readPtr) != MA_SUCCESS || avail == 0) {
                if (dst) ma_silence_pcm_frames(dst, remaining, audioFormat.toMaFormat(), audioFormat.channels);
                break;
            }
            ma_uint32 chunk = (avail < remaining) ? avail : remaining;
            if (dst) std::memcpy(dst, readPtr, chunk * frameSize);
            ma_pcm_rb_commit_read(&ring, chunk);

            dst += chunk * frameSize;
            remaining -= chunk;
            totalRead += chunk;
        }
        SI_LOG("Speaker dataCallback: framesReq=" << frames << " read=" << totalRead << " remaining=" << remaining);
    }

public:
    ma_result wakeUp() override {
        // opens playback device, sets audioFormat, calls renegotiate()
        ma_result r = AudioDevice::wakeUp();
        if (r != MA_SUCCESS) return r;

        // ring stores PCM already in consumer format (audioFormat)
        const ma_uint32 capacityFrames = audioFormat.sampleRate / 2; // ~0.5s
        r = ma_pcm_rb_init(audioFormat.toMaFormat(), audioFormat.channels, capacityFrames, nullptr, nullptr, &ring);
        ringInit = (r == MA_SUCCESS);
        SI_LOG("Speaker wakeUp: ringInit=" << ringInit << " capacityFrames=" << capacityFrames);
        return r;
    }

    void sleep() override {
        if (ringInit) { ma_pcm_rb_uninit(&ring); ringInit = false; }
        AudioDevice::sleep();
    }

    // AudioEndpoint guarantees frames here are already in `audioFormat`.
    void receivePCM(const void* frames, ma_uint32 count) override {
        if (!ringInit || !isAwake || count == 0) return;

        const uint8_t* src = static_cast<const uint8_t*>(frames);
        const size_t frameSize = audioFormat.frameSizeBytes();
        ma_uint32 remaining = count;

        ma_uint32 totalWritten = 0;
        while (remaining > 0) {
            void* writePtr = nullptr;
            ma_uint32 cap = 0;
            if (ma_pcm_rb_acquire_write(&ring, &cap, &writePtr) != MA_SUCCESS) break;

            if (cap == 0) {
                // Drop oldest data to make room (overwrite policy)
                ma_uint32 drop = std::min<ma_uint32>(remaining, count);
                ma_pcm_rb_commit_read(&ring, drop);
                continue;
            }

            ma_uint32 chunk = (cap < remaining) ? cap : remaining;
            std::memcpy(writePtr, src, chunk * frameSize);
            ma_pcm_rb_commit_write(&ring, chunk);

            src += chunk * frameSize;
            remaining -= chunk;
            totalWritten += chunk;
        }
        SI_LOG("Speaker receivePCM: in=" << count << " written=" << totalWritten << " remaining=" << remaining);
    }

    AudioSpeakerDevice(const std::string& id, ma_context* context) : AudioDevice(id, context) {}

    ma_data_source* dataSource() override { return nullptr; }
    AudioFormat format() const override { return audioFormat; }
};
