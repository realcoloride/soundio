#pragma once

#include "./AudioDevice.h"

class AudioSpeakerDevice : public AudioDevice {

public:
	ma_result wakeUp() override {
		ma_result result = MA_SUCCESS;

		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.pPlaybackDeviceID = &deviceInfo.id;
		engineConfig.channels = channels;
		engineConfig.sampleRate = sampleRate;

		engine = std::make_unique<ma_engine>();
		result = ma_engine_init(&engineConfig, engine.get());

		if (result == MA_SUCCESS)
			this->internalDevice = engine.get()->pDevice;
		
		isAwake = result == MA_SUCCESS;
		return result;
	}
	void sleep() override {
		if (engine) ma_engine_uninit(engine.get());
		if (internalDevice) internalDevice = nullptr;
		AudioDevice::sleep();
	}

	AudioSpeakerDevice(const std::string& deviceId) : AudioDevice(deviceId) {
		
	}
	~AudioSpeakerDevice() {
		
	}

	std::unique_ptr<ma_engine> engine;
};