#pragma once

#include "../include.h"
#include "../core/AudioFormat.h"
#include "../input/AudioInput.h"

class AudioOutput {
public:
    virtual ~AudioOutput() = default;
    virtual AudioFormat getPreferredFormat() const = 0;
    virtual ma_result subscribe(AudioInput& in) = 0;
    virtual void      unsubscribe(AudioInput* in) = 0;
    virtual void      unsubscribeAll() = 0;
};