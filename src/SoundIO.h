#pragma once

constexpr int SOUNDIO_VERSION = 100;

#include "./include.h"

// core
#include "./core/AudioFormat.h"

// device
#include "./device/AudioDevice.h"
#include "./device/AudioMicrophoneDevice.h"
#include "./device/AudioSpeakerDevice.h"

// input
#include "./input/AudioFileInput.h"
#include "./input/AudioStreamInput.h"

// mixer
#include "./mixer/AudioCombiner.h"
#include "./mixer/AudioPipe.h"
#include "./mixer/AudioResampler.h"

// output
#include "./output/AudioFileOutput.h"
#include "./output/AudioStreamOutput.h"

// player
#include "./player/AudioPlayer.h"

// utils
#include "./utils/deviceid.h"
#include "./utils/deviceloops.h"

class SoundIO {
private:    
    static ma_context context;

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
    template <typename T>
    static void addDevice(std::unique_ptr<T> device) {
        static_assert(std::is_base_of<AudioDevice, T>::value, "T must derive from AudioDevice");
        idToDevices.emplace(device->id, std::move(device));
    }

    static void removeDevice(const std::string& deviceId) {
        auto it = idToDevices.find(deviceId);
        if (it != idToDevices.end())
            idToDevices.erase(it); 
    }

public:
    static ma_result refreshDevices();

    static AudioDevice* getDeviceById(const std::string& id) {
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

    // unique_ptr cleanup happens automatically
    static void clearAllNodes() { nodes.clear(); }

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
    /*
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
    static AudioFileOutput* createFileOutput() {

    }
    static AudioStreamOutput* createStreamOutput() {

    }

    // player
    static AudioPlayer* createAudioPlayer(AudioInput* input) {

    }*/
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

    // track all IDs we encounter this refresh
    std::unordered_set<std::string> seenIds;

    loopDevices(context, speakers, speakerCount, ma_device_type_playback,
        [&result, &defaultSpeaker, &seenIds]
        (ma_device_info deviceInfo, std::string normalizedDeviceId, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
            seenIds.insert(normalizedDeviceId);

            auto creationCallback = [&normalizedDeviceId]() -> std::unique_ptr<AudioSpeakerDevice> {
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
        [&result, &defaultMicrophone, &seenIds]
        (ma_device_info deviceInfo, std::string normalizedDeviceId, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
            seenIds.insert(normalizedDeviceId);

            auto creationCallback = [&normalizedDeviceId]() -> std::unique_ptr<AudioMicrophoneDevice> {
                return std::make_unique<AudioMicrophoneDevice>(normalizedDeviceId);
            };

            handleDeviceLoop<AudioMicrophoneDevice>(
                deviceInfo, normalizedDeviceId, format, sampleRate, channels,
                defaultMicrophone, creationCallback
            );
        }
    );

    if (result != MA_SUCCESS) return result;

    // remove any devices that were not seen in this refresh
    for (auto it = idToDevices.begin(); it != idToDevices.end(); ) {
        if (seenIds.count(it->first) == 0)
            it = idToDevices.erase(it);
        else
            ++it;
    }

    // update defaults
    if (defaultSpeaker)
        defaultSpeakerId = defaultSpeaker->id;
    else
        defaultSpeakerId.clear();

    if (defaultMicrophone)
        defaultMicrophoneId = defaultMicrophone->id;
    else
        defaultMicrophoneId.clear();

    return MA_SUCCESS;
}
