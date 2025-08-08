#pragma once

#include "../include.h"

class AudioNode {
protected:
    std::vector<AudioNode*> connectedNodes;

    virtual ma_result canSubscribe(AudioNode* audioNode) { return MA_NOT_IMPLEMENTED; }
    virtual ma_result handleSubscribe(AudioNode*) { return MA_NOT_IMPLEMENTED; }
    virtual ma_result handleUnsubscribe(AudioNode*) { return MA_NOT_IMPLEMENTED; }

public:
    bool isSubscribed(AudioNode* audioNode) {
        return std::find(connectedNodes.begin(), connectedNodes.end(), audioNode) != connectedNodes.end();
    }

    ma_result subscribe(AudioNode* audioNode) {
        ma_result subscribeResult = canSubscribe(audioNode);
        if (subscribeResult != MA_SUCCESS)
            return subscribeResult;

        subscribeResult = audioNode->canSubscribe(this);
        if (subscribeResult != MA_SUCCESS)
            return subscribeResult;

        if (this->isSubscribed(audioNode))
            return MA_DEVICE_ALREADY_INITIALIZED;

        subscribeResult = handleSubscribe(audioNode);
        if (subscribeResult != MA_SUCCESS)
            return subscribeResult;

        connectedNodes.push_back(audioNode);

        return MA_SUCCESS;
    }

    ma_result unsubscribe(AudioNode* audioNode) {
        auto it = std::find(connectedNodes.begin(), connectedNodes.end(), audioNode);
        if (it == connectedNodes.end())
            return MA_DEVICE_NOT_INITIALIZED;

        ma_result result = handleUnsubscribe(audioNode);
        if (result != MA_SUCCESS)
            return result;

        connectedNodes.erase(it);
        return MA_SUCCESS;
    }

    void unsubscribeAll() {
        for (auto* node : connectedNodes) {
            handleUnsubscribe(node);
        }
        connectedNodes.clear();
    }
};
