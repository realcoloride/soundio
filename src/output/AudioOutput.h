#pragma once

#include "../core/AudioEndpoint.h"

class AudioInput; // forward decl

class AudioOutput : public virtual AudioEndpoint {
public:
    bool isSubscribed() override { return isInputSubscribed(); }

    ma_result subscribe(AudioInput* source);
    ma_result unsubscribe() { return unsubscribeInput(); }

    ma_uint32 getAvailableReadFrames() {
        return getRingAvailableRead(&inputRing);
    }

    virtual ~AudioOutput() = default;
};

inline ma_result AudioOutput::subscribe(AudioInput* source) {
    return subscribeInput(reinterpret_cast<AudioNode*>(source));
}
