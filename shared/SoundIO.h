#pragma once

#include <include.h>
#include <utils/deviceid.h>
#include <utils/deviceloops.h>

#include <input/AudioInput.h>
#include <input/AudioInputFile.h>
#include <input/AudioInputStream.h>
#include <input/AudioDeviceInput.h>

#include <device/AudioDevice.h>
#include <device/AudioMicrophoneDevice.h>
#include <device/AudioSpeakerDevice.h>

#include <player/AudioPlayer.h>

class SoundIO {
private:
    static inline ma_context context;


    static inline bool initialized = false;

    static inline std::map<std::string, std::unique_ptr<AudioDevice>> idToDevices;
    //static std::map<ma_device*, std::string&> deviceToId;

    static ma_result onDeviceInit(
        ma_device* pDevice, const ma_device_config* pConfig,
        ma_device_descriptor* pDescriptorPlayback, ma_device_descriptor* pDescriptorCapture
    ) { 
        return pDevice == nullptr ? MA_INVALID_ARGS : refreshDevices(); 
    }
    static ma_result onDeviceUninit(ma_device* pDevice) {
        return pDevice == nullptr ? MA_INVALID_ARGS : refreshDevices();
    }

    static inline std::string defaultCaptureDeviceId = "";
    static inline std::string defaultPlaybackDeviceId = "";

    template <typename T, typename = std::enable_if_t<std::is_base_of<AudioDevice, T>::value>>
    static void handleDeviceLoop(
        const ma_device_info& deviceInfo,
        const std::string& normalizedDeviceId,
        ma_format format,
        ma_uint32 sampleRate,
        ma_uint32 channels,
        T*& defaultDevice,
        std::function<std::unique_ptr<T>()> creationCallback
    ) {
        // step 1: check if the device already exists
        T* device = dynamic_cast<T*>(getDeviceById(normalizedDeviceId));

        // step 2: if the device doesn't exist, invoke the creation callback
        if (device == nullptr) {
            auto newDevice = creationCallback(); // create the device
            if (newDevice == nullptr) return;

            device = newDevice.get();
            addDevice(std::move(newDevice));
        }

        // step 3: update the device's details
        device->updateDevice(deviceInfo, format, sampleRate, channels);

        // step 4: handle default device logic
        if (device->isDefault)
            defaultDevice = device;
    }

public:
    static ma_result initialize();
    static ma_result shutdown() {
        if (!initialized) return MA_SUCCESS;


        return MA_SUCCESS;
    }

protected:
    static void addDevice(std::unique_ptr<AudioSpeakerDevice> device) {
        std::string deviceId = device->id;
        idToDevices[deviceId] = std::move(device);
    }
    static void removeDevice(const std::string& deviceId) {
        auto it = idToDevices.find(deviceId);
        if (it != idToDevices.end())
            idToDevices.erase(it); 
    }

public:
    static ma_result refreshDevices();

    static AudioDevice* getDeviceById(std::string id) {
        auto it = idToDevices.find(id);
        return it != idToDevices.end() ? it->second.get() : nullptr;
    }
    static auto getDevices() {
        // return devices;
    }

    static AudioMicrophoneDevice* getDefaultCaptureDevice() {
        return defaultCaptureDeviceId.empty() ? nullptr : dynamic_cast<AudioMicrophoneDevice*>(getDeviceById(defaultCaptureDeviceId));
    }
    static AudioSpeakerDevice* getDefaultPlaybackDevice() {
        return defaultPlaybackDeviceId.empty() ? nullptr : dynamic_cast<AudioSpeakerDevice*>(getDeviceById(defaultPlaybackDeviceId));
    }
    

    /*static Napi::Value getDefaultInputDevice() {

    }
    static Napi::Value getDefaultOutputDevice() {

    }

    static Napi::Value startDeviceMonitoring(const Napi::CallbackInfo& info);
    static Napi::Value stopDeviceMonitoring(const Napi::CallbackInfo& info);
    static Napi::Value onDeviceChanged(const Napi::CallbackInfo& info);
    static Napi::Value onDeviceDisconnected(const Napi::CallbackInfo& info);

    static Napi::Value isPlaybackSupported(const Napi::CallbackInfo& info);
    static Napi::Value isRecordingSupported(const Napi::CallbackInfo& info);
    */
};
