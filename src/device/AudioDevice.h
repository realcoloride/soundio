#pragma once

#include "../include.h"
#include "../core/AudioNode.h"

class AudioDevice {
protected:
	ma_device* internalDevice;
	std::unique_ptr<ma_engine> engine;

	virtual void dataCallback(
		ma_device* pDevice, 
		void* pOutput, 
		const void* pInput, 
		ma_uint32 frameCount
	) { (void)pOutput; }

public:
	std::string id;
	std::string name;

	AudioFormat format;
	ma_device_info deviceInfo;

	bool isDefault = false;

	bool isAwake = false;
	virtual ma_result wakeUp() {
		ma_result result = MA_SUCCESS;

		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.pPlaybackDeviceID = &deviceInfo.id;
		engineConfig.channels = channels;
		engineConfig.sampleRate = sampleRate;
		engineConfig.dataCallback = dataCallback;

		this->engine = std::make_unique<ma_engine>();
		result = ma_engine_init(&engineConfig, engine.get());

		if (result == MA_SUCCESS)
			this->internalDevice = engine.get()->pDevice;

		this->isAwake = result == MA_SUCCESS;

		return result;
	}
	void sleep() override {
		if (engine) ma_engine_uninit(engine.get());
		if (internalDevice) internalDevice = nullptr;
		AudioDevice::sleep();
	}
	
	ma_result ensureAwake() { return !this->isAwake ? wakeUp() : MA_SUCCESS; }

	void updateDevice(ma_device_info deviceInfo, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
		this->name = deviceInfo.name ? std::string(deviceInfo.name) : std::string{};

		this->deviceInfo = deviceInfo;
		// ensure proper bool conversion from ma_bool32
		this->isDefault = (deviceInfo.isDefault != 0);
		this->format = AudioFormat(format, sampleRate, channels);
	}

	AudioDevice(std::string deviceId) { this->id = deviceId; }
	virtual ~AudioDevice() { this->sleep(); }

};