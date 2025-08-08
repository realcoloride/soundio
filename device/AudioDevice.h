#pragma once

#include <include.h>

class AudioDevice {
public:
	std::string id;
	std::string name;

	ma_device_info deviceInfo;
	ma_format format;
	ma_uint32 sampleRate;
	ma_uint32 channels;
	bool isDefault;

	ma_device* internalDevice;


	void subscribe() {
		
	}
	void unsubscribe() {

	}
	void unsubscribeAll() {

	}
	
	void queueBufferForInputs() {

	}


	bool isAwake;
	virtual ma_result wakeUp() {
		isAwake = true;
		return MA_SUCCESS;
	}
	virtual void sleep() { isAwake = false; }
	ma_result ensureAwake() { 
		return !isAwake ? wakeUp() : MA_SUCCESS;
	}

	void updateDevice(ma_device_info deviceInfo, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
		std::string deviceNameString(deviceInfo.name);

		this->name = deviceNameString;
		this->isDefault = deviceInfo.isDefault;
		this->format = format;
		this->sampleRate = sampleRate;
		this->channels = channels;
	}

	AudioDevice(std::string deviceId) {
		this->id = deviceId;
	}
	virtual ~AudioDevice() {
		sleep();
	}
};