#pragma once

#include "../include.h"
#include "./AudioNode.h"

class AudioNode {
protected:
    using IsSubscribedMethod = bool (AudioNode::*)();
    using HandleMethod = ma_result(AudioNode::*)(AudioNode*);

    AudioNode* inputNode = nullptr;
    AudioNode* outputNode = nullptr;

    virtual ma_result handleInputSubscribe(AudioNode*) { return MA_SUCCESS; }
    virtual ma_result handleOutputSubscribe(AudioNode*) { return MA_SUCCESS; }
    virtual ma_result handleInputUnsubscribe(AudioNode*) { return MA_SUCCESS; }
    virtual ma_result handleOutputUnsubscribe(AudioNode*) { return MA_SUCCESS; }

    bool isInputSubscribed() { return inputNode != nullptr; }
    bool isOutputSubscribed() { return outputNode != nullptr; }
    bool areBothSubscribed() { return isInputSubscribed() && isOutputSubscribed(); }

    AudioFormat audioFormat;

    ma_result _subscribe(
        AudioNode*& audioNode,
        IsSubscribedMethod isSubscribedMethod,
        HandleMethod handleMethod,
        AudioNode* otherNode,
        HandleMethod otherHandleMethod
    ) {
        if ((this->*isSubscribedMethod)())
            return MA_DEVICE_ALREADY_INITIALIZED;

        SI_LOG("subscribe begin: this=" << this << ", other=" << otherNode);
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

        SI_LOG("subscribe done: inputNode=" << inputNode << ", outputNode=" << outputNode);
        return MA_SUCCESS;
    }

    ma_result _unsubscribe(
        AudioNode*& audioNode,
        IsSubscribedMethod isSubscribedMethod,
        HandleMethod handleMethod
    ) {
        if (!(this->*isSubscribedMethod)())
            return MA_DEVICE_NOT_INITIALIZED;

        SI_LOG("unsubscribe begin: this=" << this << ", peer=" << audioNode);
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
        SI_LOG("unsubscribe done: inputNode=" << inputNode << ", outputNode=" << outputNode);
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
    
    void unsubscribeAll() {
        this->unsubscribeInput();
        this->unsubscribeOutput();
    }

    virtual ma_data_source* dataSource() = 0;
    virtual AudioFormat format() const = 0;

public:
    ~AudioNode() { SI_LOG("~AudioNode(" << this << ")"); this->unsubscribeAll(); }
};
