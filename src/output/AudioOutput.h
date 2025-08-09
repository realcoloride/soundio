#pragma once
#include "../core/AudioNode.h"
#include "../core/AudioFormat.h"

class AudioOutput : public virtual AudioNode {
protected:
    AudioFormat audioFormat;

public:
    AudioOutput(AudioFormat audioFormat)
        : audioFormat(audioFormat) {
    }

    // called by downstream consumers (speakers, file writers, etc.)
    virtual void readPCM(void* outputBuffer, ma_uint32 frameCount) = 0;

    const AudioFormat& getFormat() const { return audioFormat; }
};
