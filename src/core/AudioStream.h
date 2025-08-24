#pragma once

#include "./AudioEndpoint.h"

class AudioStream : public virtual AudioEndpoint {
protected:
    AudioStream(const AudioFormat& fmt, bool isSource, bool isSink) {
        audioFormat = fmt;
        canFillInputRing = isSink;
        canDrainOutputRing = isSource;
    }

    void pushToOutputRing(const void* pData, ma_uint32 frameCount) {
        writeRing(outputRing, outputRingFormat, pData, frameCount);
    }

    ma_uint32 pullFromInputRing(void* pOut, ma_uint32 frameCount) {
        return readRing(inputRing, inputRingFormat, pOut, frameCount);
    }
};
