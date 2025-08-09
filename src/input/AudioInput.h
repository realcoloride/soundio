#pragma once

#include "../core/AudioEndpoint.h"

class AudioOutput;

class AudioInput : public virtual AudioEndpoint {
public:
    bool isSubscribed() override { return isOutputSubscribed(); }

    ma_result subscribe(AudioOutput* destination);
    ma_result unsubscribe() { return unsubscribeOutput(); }

    virtual ~AudioInput() = default;
};

inline ma_result AudioInput::subscribe(AudioOutput* destination) {
    return subscribeOutput((AudioNode*)destination);
}
