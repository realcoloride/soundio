#pragma once

#include "../core/AudioStream.h"
#include "AudioOutput.h"

class AudioStreamOutput: public AudioStream, public virtual AudioOutput {
public:
    AudioStreamOutput(const AudioFormat& format) : AudioStream(format, false, true) {}
    ma_uint32 receivePCM(void* pOut, ma_uint32 frameCount) { return pullFromInputRing(pOut, frameCount); }
};