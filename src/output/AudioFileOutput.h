#pragma once

#include "../core/AudioFile.h"

class AudioFileOutput : public AudioFile, public virtual AudioOutput {
public:
    AudioFileOutput() : AudioFile(false, true) {}

    ma_result open(const std::string& filePath, const AudioFormat& targetFormat) {
        return openEncoder(filePath, targetFormat);
    }

    void close() {
        closeEncoder();
    }

    ma_uint32 receivePCM(void* pOut, ma_uint32 frameCount) {
        return pullFromInputRing(pOut, frameCount);
    }
};
