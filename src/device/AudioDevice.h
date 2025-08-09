#pragma once

#include "../include.h"
#include "../core/AudioEndpoint.h"

class AudioDevice : public virtual AudioEndpoint {
protected:
	ma_device* internalDevice = nullptr;
	std::unique_ptr<ma_engine> engine;

	virtual void dataCallback(
		ma_device* pDevice, 
		void* pOutput, 
		const void* pInput, 
		ma_uint32 frameCount
	) { (void)pOutput; }
	
	static void onDeviceData(ma_device* device, void* out, const void* in, ma_uint32 frames) {
		auto* self = static_cast<AudioDevice*>(device->pUserData);
		if (!self) return;
		self->dataCallback(device, out, in, frames);  // calls your virtual override
	}

public:
	std::string id;
	std::string name;

	AudioFormat deviceFormat;
	ma_device_info deviceInfo;

	bool isDefault = false;

	bool isAwake = false;
	virtual ma_result wakeUp() {
		ma_result result = MA_SUCCESS;

		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.pContext = &SoundIO::context;
		engineConfig.pPlaybackDeviceID = &deviceInfo.id;
		engineConfig.channels = deviceFormat.channels;
		engineConfig.sampleRate = deviceFormat.sampleRate;
		engineConfig.dataCallback = &AudioDevice::onDeviceData;
		engineConfig.pUserData = this;

		this->engine = std::make_unique<ma_engine>();
		result = ma_engine_init(&engineConfig, engine.get());

		if (result == MA_SUCCESS) {
			this->internalDevice = engine.get()->pDevice;
			this->isAwake = true;
			this->audioFormat = deviceFormat;
			this->renegotiate();
		} else {
			this->internalDevice == nullptr;
			engine.reset();
		}

		return result;
	}
	void sleep() {
		if (internalDevice) internalDevice = nullptr;
		if (engine) {
			ma_engine_uninit(engine.get());
			engine.reset();
		}
		isAwake = false;
	}
	
	ma_result ensureAwake() { return !this->isAwake ? wakeUp() : MA_SUCCESS; }

	void updateDevice(ma_device_info deviceInfo, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
		this->name = deviceInfo.name ? std::string(deviceInfo.name) : std::string{};
		this->deviceInfo = deviceInfo;
		this->isDefault = (deviceInfo.isDefault != 0);

		AudioFormat newFormat(format, channels, sampleRate);
		if (deviceFormat != newFormat) {
			this->deviceFormat = newFormat;
			this->audioFormat = newFormat;
			this->renegotiate();
		}
	}

	AudioDevice(std::string deviceId) { this->id = deviceId; }
	virtual ~AudioDevice() { this->sleep(); }

	AudioDevice(const AudioDevice&) = delete;
	AudioDevice& operator=(const AudioDevice&) = delete;
	AudioDevice(AudioDevice&&) = delete;
	AudioDevice& operator=(AudioDevice&&) = delete;
};