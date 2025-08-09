#pragma once

#include "../core/AudioEndpoint.h"

class AudioOutput : public virtual AudioEndpoint {
public:
    bool isSubscribed() override { return isInputSubscribed(); }

    ma_result subscribe(AudioInput* source) { return subscribeInput(static_cast<AudioNode*>(source)); }
    ma_result unsubscribe() { return unsubscribeInput(); }

    virtual ~AudioOutput() = default;
};
