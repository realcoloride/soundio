#pragma once

#include "../core/AudioEndpoint.h"

class AudioInput : public AudioEndpoint {
public:
    // called by upstream producers (microphones, files, streams)
    virtual void submitPCM(const void* data, ma_uint32 frameCount) = 0;

    // by default, AudioInput only outputs PCM data to whatever is subscribed
    bool isSubscribed() override { return isOutputSubscribed(); }

    ma_result subscribe(AudioOutput* destinationNode) { return subscribeOutput(destinationNode); }
    ma_result unsubscribe() { return unsubscribeOutput(); }
};
