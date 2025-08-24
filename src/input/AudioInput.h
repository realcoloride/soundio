#pragma once

#include "../core/AudioEndpoint.h"
#include "../output/AudioOutput.h" 

class AudioOutput; // forward decl

class AudioInput : public virtual AudioEndpoint {
public:
    bool isSubscribed() override { return isOutputSubscribed(); }

    ma_result subscribe(AudioOutput* destination);
    ma_result unsubscribe() { return unsubscribeOutput(); }

    virtual ~AudioInput() = default;
    
    ma_uint32 getAvailableWriteFrames() {
        return getRingAvailableWrite(&outputRing);
    }
};

inline ma_result AudioInput::subscribe(AudioOutput* destination) {
    return subscribeOutput(static_cast<AudioNode*>(destination));
}
