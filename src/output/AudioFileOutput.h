#pragma once

#include "../core/AudioFile.h"
#include "../output/AudioOutput.h"

class AudioFileOutput : public AudioFile, public virtual AudioOutput {
protected:
    void whenInputSubmitted(const void*, ma_uint32) override {
        if (!hasEncoder) return;

        ma_pcm_rb ring = AudioFile::inputRing;
        const ma_uint32 available = ma_pcm_rb_available_read(&ring);

        if (available > 0) {
            std::vector<uint8_t> buffer(AudioEndpoint::inputRingFormat.frameSizeInBytes(available));

            ma_uint32 framesRead = AudioEndpoint::readRing(
                AudioEndpoint::inputRing,
                AudioEndpoint::inputRingFormat,
                buffer.data(),
                available
            );

            if (framesRead > 0)
                bufferStatus = writeToFile(buffer.data(), framesRead);
        }
    }

public:
    AudioFileOutput() : AudioFile(false, true) {}

    ma_result open(const std::string& filePath, const AudioFormat& targetFormat) {
        return openEncoder(filePath, targetFormat);
    }

    void close() {
        closeEncoder();
    }

    bool isOpen() const { return isEncoderOpen(); }
};