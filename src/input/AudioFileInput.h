#pragma once

#include "../core/AudioFile.h"

class AudioFileInput : public AudioFile, public virtual AudioInput {
public:
    AudioFileInput() : AudioFile(true, false) {}

    ma_result open(const std::string& filePath) {
        return openDecoder(filePath);
    }

    void close() {
        closeDecoder();
    }

    void submitPCM(const void* pData, ma_uint32 frameCount) {
        pushToOutputRing(pData, frameCount);
    }

    void whenOutputSubmitted() override {
        if (!hasDecoder) return;

        const ma_uint32 chunkFrames = 512;
        std::vector<uint8_t> buffer(audioFormat.frameSizeInBytes(chunkFrames));
        ma_uint32 framesRead;
        if (readFromFile(buffer.data(), chunkFrames, &framesRead) == MA_SUCCESS && framesRead > 0) {
            pushToOutputRing(buffer.data(), framesRead);
        }
    }
};
