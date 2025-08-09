#pragma once

#include "../core/AudioEndpoint.h"

class AudioInput : public virtual AudioEndpoint {
public:
    // by default, AudioInput only outputs PCM data to whatever is subscribed
    bool isSubscribed() override { return isOutputSubscribed(); }

    ma_result subscribe(AudioOutput* destinationNode) { return subscribeOutput(destinationNode); }
    ma_result unsubscribe() { return unsubscribeOutput(); }
};
