#pragma once

#include "../core/AudioFile.h"
#include "../input/AudioInput.h"

class AudioFileInput : public AudioFile, public virtual AudioInput {
protected:
    void whenOutputSubmitted(void*, ma_uint32 frameCount) override {
        if (!hasDecoder) return;

        std::vector<uint8_t> buffer(audioFormat.frameSizeInBytes(frameCount));
        ma_uint32 framesRead = 0;
        
        bufferStatus = readFromFile(buffer.data(), frameCount, &framesRead);
        if (bufferStatus == MA_SUCCESS && framesRead > 0) {
            receivePCM(buffer.data(), framesRead);
            mixPCM();
        }
    }

    void whenRenegotiated() override { openDecoder(); }

public:
    AudioFileInput() : AudioFile(true, true) {}

    ma_result open(const std::string& filePath) { return prepareOpen(filePath, &checkDecode); }
    void close() { closeDecoder(); }
    bool isOpen() const { return isDecoderOpen(); }
};
