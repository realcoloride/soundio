#pragma once

#include "include.h"
#include "utils/deviceid.h"
#include "utils/deviceloops.h"

#include "input/AudioInput.h"
#include "input/AudioInputFile.h"
#include "input/AudioInputStream.h"
#include "input/AudioDeviceInput.h"

#include "device/AudioDevice.h"
#include "device/AudioMicrophoneDevice.h"
#include "device/AudioSpeakerDevice.h"

#include "player/AudioPlayer.h"

#define SOUNDIO_VERSION 100

class SoundIO {
private:
    static inline ma_context context;
    static inline bool initialized = false;

    static inline std::map<std::string, std::unique_ptr<AudioDevice>> idToDevices;

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
        T* device = dynamic_cast<T*>(getDeviceById(normalizedDeviceId));

        if (device == nullptr) {
            auto newDevice = creationCallback();
            if (newDevice == nullptr) return;

            device = newDevice.get();
            addDevice(std::move(newDevice));
        }

        device->updateDevice(deviceInfo, format, sampleRate, channels);

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
        // TODO: return container if needed
    }

    static AudioMicrophoneDevice* getDefaultCaptureDevice() {
        return defaultCaptureDeviceId.empty() ? nullptr : dynamic_cast<AudioMicrophoneDevice*>(getDeviceById(defaultCaptureDeviceId));
    }

    static AudioSpeakerDevice* getDefaultPlaybackDevice() {
        return defaultPlaybackDeviceId.empty() ? nullptr : dynamic_cast<AudioSpeakerDevice*>(getDeviceById(defaultPlaybackDeviceId));
    }
};

inline ma_result SoundIO::initialize() {
    if (initialized) return MA_NO_MESSAGE;

    // initialize miniaudio context with default backends
    ma_result result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) return result;
    if (context.backend == ma_backend_null) return MA_BACKEND_NOT_ENABLED;

    // setup callbacks
    context.callbacks.onDeviceInit = onDeviceInit;
    context.callbacks.onDeviceUninit = onDeviceUninit;

    refreshDevices();

    initialized = true;
    return result;
}

inline ma_result SoundIO::refreshDevices() {
    ma_device_info* playbackDevices;
    ma_uint32 playbackCount;
    ma_device_info* captureDevices;
    ma_uint32 captureCount;

    ma_context_get_devices(&context, &playbackDevices, &playbackCount, &captureDevices, &captureCount);

    ma_result result = MA_SUCCESS;

    AudioMicrophoneDevice* defaultCaptureDevice = nullptr;
    AudioSpeakerDevice* defaultPlaybackDevice = nullptr;

    idToDevices.clear();

    loopDevices(context, playbackDevices, playbackCount, ma_device_type_playback,
        [&result, &defaultPlaybackDevice]
        (ma_device_info deviceInfo, std::string normalizedDeviceId, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
            auto creationCallback = [&normalizedDeviceId, &result, &deviceInfo, channels, sampleRate]() -> std::unique_ptr<AudioSpeakerDevice> {
                return std::make_unique<AudioSpeakerDevice>(normalizedDeviceId);
            };

            handleDeviceLoop<AudioSpeakerDevice>(
                deviceInfo, normalizedDeviceId, format, sampleRate, channels,
                defaultPlaybackDevice, creationCallback
            );
        }
    );

    if (result != MA_SUCCESS) return result;

    // TODO handling microphones soon!

    if (result != MA_SUCCESS) return result;

    // reset if the default devices no longer exist
    if (defaultPlaybackDevice)
        defaultPlaybackDeviceId = defaultPlaybackDevice->id;
    if (defaultCaptureDevice)
        defaultCaptureDeviceId = defaultCaptureDevice->id;

    return result;
}