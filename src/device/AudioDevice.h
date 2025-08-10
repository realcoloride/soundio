#pragma once

#include "../include.h"
#include "../core/AudioEndpoint.h"

class AudioDevice : public virtual AudioEndpoint {
protected:
	std::unique_ptr<ma_device> device = nullptr;
	ma_context* context = nullptr;

	ma_uint32 pullFromEndpoint(AudioEndpoint* ep, void* pOut, ma_uint32 frames) {
		return ep->submitPCM(pOut, frames);
	}

	virtual void dataCallback(
		ma_device* pDevice, 
		void* pOutput, 
		const void* pInput, 
		ma_uint32 frameCount
	) { (void)pOutput; }
	
	static void onDeviceData(ma_device* device, void* out, const void* in, ma_uint32 frames) {
		auto* self = static_cast<AudioDevice*>(device->pUserData);
		SI_LOG("onDeviceData called for: " << self->name << ", type=" << self->deviceType);
		if (!self) return;
		self->dataCallback(device, out, in, frames);  // calls your virtual override
	}

public:
	std::string id;
	std::string name;

	AudioFormat deviceFormat;
	ma_device_info deviceInfo;
	ma_device_type deviceType;

	bool isDefault = false;

	bool isAwake = false;
	virtual ma_result wakeUp() {
		ma_result result = MA_SUCCESS;
		this->isAwake = false;
		SI_LOG("wakeUp: context=" << context << " backend=" << (context ? context->backend : -999));

		ma_device_config config = ma_device_config_init(deviceType);
		if (deviceType == ma_device_type_capture) {
			config.capture.pDeviceID = &deviceInfo.id;
			config.capture.format = deviceFormat.format;
			config.capture.channels = deviceFormat.channels;
		}
		else if (deviceType == ma_device_type_playback) {
			config.playback.pDeviceID = &deviceInfo.id;
			config.playback.format = deviceFormat.format;
			config.playback.channels = deviceFormat.channels;
		}

		config.sampleRate = deviceFormat.sampleRate;
		config.dataCallback = &AudioDevice::onDeviceData;
		config.pUserData = this;

		this->device = std::make_unique<ma_device>();

		result = ma_device_init(context, &config, device.get());
		
		if (result != MA_SUCCESS) {
			device.reset();
			SI_LOG("wakeUp FAILED: id=" << id << " res=" << result);
			return result;
		}

		result = ma_device_start(device.get());
		SI_LOG("THIS SHOULD BE SEEN");
		if (result == MA_SUCCESS) {
			this->isAwake = true;
			this->audioFormat = deviceFormat;
			this->renegotiate();
			SI_LOG("wakeUp ok: id=" << id << " name=" << name << " fmt=" << deviceFormat.format << " ch=" << deviceFormat.channels << " sr=" << deviceFormat.sampleRate);
		} else {
			ma_device_uninit(device.get());
			device.reset();

			SI_LOG("wakeUp FAILED: id=" << id << " res=" << result);
		}

		return result;
	}
	virtual void sleep() {
		if (!isAwake) return;

		SI_LOG("sleep: id=" << id);
		if (device) {
			ma_device_uninit(device.get());
			device.reset();
		}
		isAwake = false;
	}
	
	ma_result ensureAwake() {
		SI_LOG("ensureAwake called for " << name << ", isAwake=" << isAwake);  return !this->isAwake ? wakeUp() : MA_SUCCESS; }

	void updateDevice(ma_device_info deviceInfo, ma_device_type deviceType, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
		this->name = deviceInfo.name ? std::string(deviceInfo.name) : std::string{};
		this->deviceInfo = deviceInfo;
		this->isDefault = (deviceInfo.isDefault != 0);
		this->deviceType = deviceType;

		AudioFormat newFormat(format, channels, sampleRate);
		if (deviceFormat != newFormat) {
			this->deviceFormat = newFormat;
			this->audioFormat = newFormat;
			this->renegotiate();
			SI_LOG("updateDevice: id=" << id << " default=" << isDefault << " fmt=" << format << " ch=" << channels << " sr=" << sampleRate);
		}
	}

	AudioDevice(std::string deviceId, ma_context* context) { 
		this->id = deviceId;
		this->context = context;
	}
	virtual ~AudioDevice() { this->sleep(); }

	AudioDevice(const AudioDevice&) = delete;
	AudioDevice& operator=(const AudioDevice&) = delete;
	AudioDevice(AudioDevice&&) = delete;
	AudioDevice& operator=(AudioDevice&&) = delete;
};