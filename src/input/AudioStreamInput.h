#pragma once

#include "../core/AudioStream.h"
#include "AudioInput.h"

class AudioStreamInput : public AudioStream, public virtual AudioInput {
public:
    AudioStreamInput(const AudioFormat& format) : AudioStream(format, true, false) {}
    void submitPCM(const void* pData, ma_uint32 frameCount) { pushToOutputRing(pData, frameCount); }
};