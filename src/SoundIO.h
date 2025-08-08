#pragma once

#define SOUNDIO_VERSION constexpr 100

#include "./include.h"

// core
#include "./core/AudioFormat.h"

// device
#include "./device/AudioDevice.h"
#include "./device/AudioMicrophoneDevice.h"
#include "./device/AudioSpeakerDevice.h"

// input
#include "./input/AudioInput.h"
#include "./input/AudioFileInput.h"
#include "./input/AudioStreamInput.h"
#include "./input/AudioDeviceInput.h"

// mixer
#include "./mixer/AudioCombiner.h"
#include "./mixer/AudioPipe.h"
#include "./mixer/AudioResampler.h"

// output
#include "./output/AudioOutput.h"
#include "./output/AudioDeviceOutput.h"
#include "./output/AudioFileOutput.h"
#include "./output/AudioStreamOutput.h"

// player
#include "./player/AudioPlayer.h"

// utils
#include "./utils/deviceid.h"
#include "./utils/deviceloops.h"

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

    static inline std::string defaultMicrophoneId = "";
    static inline std::string defaultSpeakerId = "";

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

    static std::vector<AudioDevice*> getAllDevices() {
        std::vector<AudioDevice*> out;
        out.reserve(idToDevices.size());
        for (auto& kv : idToDevices)
            out.push_back(kv.second.get());
        return out;
    }

    static std::vector<AudioMicrophoneDevice*> getAllMicrophones() {
        std::vector<AudioMicrophoneDevice*> out;
        out.reserve(idToDevices.size());
        for (auto& kv : idToDevices)
        if (auto* mic = dynamic_cast<AudioMicrophoneDevice*>(kv.second.get()))
            out.push_back(mic);
        return out;
    }

    static std::vector<AudioSpeakerDevice*> getAllSpeakers() {
        std::vector<AudioSpeakerDevice*> out;
        out.reserve(idToDevices.size());
        for (auto& kv : idToDevices)
        if (auto* spk = dynamic_cast<AudioSpeakerDevice*>(kv.second.get())) 
            out.push_back(spk);
        return out;
    }

    static AudioMicrophoneDevice* getDefaultMicrophone() {
        return defaultMicrophoneId.empty() ? nullptr : dynamic_cast<AudioMicrophoneDevice*>(getDeviceById(defaultMicrophoneId));
    }

    static AudioSpeakerDevice* getDefaultSpeaker() {
        return defaultSpeakerId.empty() ? nullptr : dynamic_cast<AudioSpeakerDevice*>(getDeviceById(defaultSpeakerId));
    }

protected:
    static inline std::vector<std::unique_ptr<AudioNode>> nodes;

    static void clearAllNodes() {
        nodes.clear(); // unique_ptr cleanup happens automatically
    }

    template <typename T, typename... Args>
    static T* registerNode(Args&&... args) {
        static_assert(std::is_base_of<AudioNode, T>::value,
            "T must derive from AudioNode");

        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = ptr.get();
        nodes.push_back(std::move(ptr)); // nodes = std::vector<std::unique_ptr<AudioNode>>
        return raw;
    }

public:

    // creating inputs, outputs etc
    // input
    static AudioDeviceInput* createDeviceInput(AudioMicrophoneDevice* microphone) {
        auto* input = registerNode<AudioDeviceInput>();
        microphone->subscribe(input);
        return input;
    }
    static AudioFileInput* createFileInput(const std::string& path) {
        auto* input = registerNode<AudioFileInput>();
        input->open(path);
        // todo
        return input;
    }
    static AudioStreamInput* createStreamInput(AudioFormat format) {
        auto* input = registerNode<AudioStreamInput>();
        // todo
        return input;
    }

    // mixer
    static AudioCombiner* createCombiner(AudioInput* input = nullptr, AudioOutput* device = nullptr) {
        
    }
    static AudioResampler* createResampler(AudioFormat outputFormat, AudioInput* input = nullptr) {

    }
    static AudioPipe* createPipe() {

    }

    // output
    static AudioDeviceOutput* createDeviceOutput(AudioSpeakerDevice* speaker) {

    }
    static AudioFileOutput* createFileOutput() {

    }
    static AudioStreamOutput* createStreamOutput() {

    }

    // player
    static AudioPlayer* createAudioPlayer() {

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
    ma_device_info* speakers;
    ma_uint32 speakerCount;
    ma_device_info* microphones;
    ma_uint32 microphoneCount;

    ma_context_get_devices(&context, &speakers, &speakerCount, &microphones, &microphoneCount);

    ma_result result = MA_SUCCESS;

    AudioMicrophoneDevice* defaultMicrophone = nullptr;
    AudioSpeakerDevice* defaultSpeaker = nullptr;

    idToDevices.clear();

    loopDevices(context, speakers, speakerCount, ma_device_type_playback,
        [&result, &defaultSpeaker]
        (ma_device_info deviceInfo, std::string normalizedDeviceId, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
            auto creationCallback = [&normalizedDeviceId, &result, &deviceInfo, channels, sampleRate]() -> std::unique_ptr<AudioSpeakerDevice> {
                return std::make_unique<AudioSpeakerDevice>(normalizedDeviceId);
            };

            handleDeviceLoop<AudioSpeakerDevice>(
                deviceInfo, normalizedDeviceId, format, sampleRate, channels,
                defaultSpeaker, creationCallback
            );
        }
    );

    if (result != MA_SUCCESS) return result;

    loopDevices(context, microphones, microphoneCount, ma_device_type_capture,
        [&result, &defaultMicrophone]
        (ma_device_info deviceInfo, std::string normalizedDeviceId, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
            auto creationCallback = [&normalizedDeviceId, &result, &deviceInfo, channels, sampleRate]() -> std::unique_ptr<AudioMicrophoneDevice> {
                return std::make_unique<AudioMicrophoneDevice>(normalizedDeviceId);
            };

            handleDeviceLoop<AudioMicrophoneDevice>(
                deviceInfo, normalizedDeviceId, format, sampleRate, channels,
                defaultMicrophone, creationCallback
            );
        }
    );

    if (result != MA_SUCCESS) return result;

    // reset if the default devices no longer exist
    if (defaultSpeaker)
        defaultSpeakerId = defaultSpeaker->id;
    if (defaultMicrophone)
        defaultMicrophoneId = defaultMicrophone->id;

    return result;
}