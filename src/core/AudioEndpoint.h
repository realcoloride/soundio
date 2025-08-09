#pragma once

#include "./AudioNode.h"
#include "./AudioFormat.h"

class AudioEndpoint : public AudioNode {
protected:
    AudioFormat audioFormat;

public:
    AudioEndpoint(AudioFormat format): audioFormat(format) {}
    const AudioFormat& getFormat() const { return audioFormat; }
};