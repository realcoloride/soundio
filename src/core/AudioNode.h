#pragma once

#include "../include.h"
#include "./AudioNode.h"

class AudioNode {
protected:
    using IsSubscribedMethod = bool (AudioNode::*)();
    using HandleMethod = ma_result(AudioNode::*)(AudioNode*);

    inline AudioNode* inputNode = nullptr;
    inline AudioNode* outputNode = nullptr;

    virtual ma_result handleInputSubscribe(AudioNode*) { return MA_NOT_IMPLEMENTED; }
    virtual ma_result handleOutputSubscribe(AudioNode*) { return MA_NOT_IMPLEMENTED; }
    virtual ma_result handleInputUnsubscribe(AudioNode*) { return MA_NOT_IMPLEMENTED; }
    virtual ma_result handleOutputUnsubscribe(AudioNode*) { return MA_NOT_IMPLEMENTED; }

    bool isInputSubscribed() { return inputNode != nullptr; }
    bool isOutputSubscribed() { return outputNode != nullptr; }

    ma_result _subscribe(
        AudioNode*& audioNode,
        IsSubscribedMethod isSubscribedMethod,
        HandleMethod handleMethod,
        AudioNode* otherNode,
        HandleMethod otherHandleMethod
    ) {
        if ((this->*isSubscribedMethod)())
            return MA_DEVICE_ALREADY_INITIALIZED;

        ma_result result = (this->*handleMethod)(otherNode);
        if (result != MA_SUCCESS)
            return result;

        if (otherNode) {
            (otherNode->*otherHandleMethod)(this);
        }

        audioNode = otherNode;
        if (otherNode) {
            if (&audioNode == &inputNode)
                otherNode->outputNode = this;
            else
                otherNode->inputNode = this;
        }

        return MA_SUCCESS;
    }

    ma_result _unsubscribe(
        AudioNode*& audioNode,
        IsSubscribedMethod isSubscribedMethod,
        HandleMethod handleMethod
    ) {
        if (!(this->*isSubscribedMethod)())
            return MA_DEVICE_NOT_INITIALIZED;

        ma_result result = (this->*handleMethod)(audioNode);
        if (result != MA_SUCCESS)
            return result;

        if (audioNode) {
            if (audioNode->outputNode == this)
                audioNode->unsubscribeOutput();
            if (audioNode->inputNode == this)
                audioNode->unsubscribeInput();
        }

        audioNode = nullptr;
        return MA_SUCCESS;
    }

    ma_result subscribeInput(AudioNode* source) {
        return _subscribe(inputNode,
            &AudioNode::isInputSubscribed,
            &AudioNode::handleInputSubscribe,
            source,
            &AudioNode::handleOutputSubscribe);
    }

    ma_result subscribeOutput(AudioNode* destination) {
        return _subscribe(outputNode,
            &AudioNode::isOutputSubscribed,
            &AudioNode::handleOutputSubscribe,
            destination,
            &AudioNode::handleInputSubscribe);
    }

    ma_result unsubscribeInput() {
        return _unsubscribe(inputNode,
            &AudioNode::isInputSubscribed,
            &AudioNode::handleInputUnsubscribe);
    }

    ma_result unsubscribeOutput() {
        return _unsubscribe(outputNode,
            &AudioNode::isOutputSubscribed,
            &AudioNode::handleOutputUnsubscribe);
    }

    virtual bool isSubscribed() { return false; }
    virtual ma_result subscribe(AudioNode*) { return MA_NOT_IMPLEMENTED; }
    virtual ma_result unsubscribe() { return MA_NOT_IMPLEMENTED; }

    void unsubscribeAll() {
        this->unsubscribeInput();
        this->unsubscribeOutput();
    }

    ~AudioNode() { this->unsubscribeAll(); }
};
