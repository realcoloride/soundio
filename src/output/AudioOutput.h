#pragma once

#include "../core/AudioEndpoint.h"

class AudioInput; // forward decl

class AudioOutput : public virtual AudioEndpoint {
public:
    bool isSubscribed() override { return isInputSubscribed(); }

    ma_result subscribe(AudioInput* source);
    ma_result unsubscribe() { return unsubscribeInput(); }

    virtual ~AudioOutput() = default;
};

inline ma_result AudioOutput::subscribe(AudioInput* source) {
    SI_LOG("AudioOutput::subscribe <- " << source);
    return subscribeInput(reinterpret_cast<AudioNode*>(source));
}
