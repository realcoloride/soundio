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
#include "./mixer/AudioAnalyzer.h"
#include "./mixer/AudioCombiner.h"

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
    static inline ma_context context;
    static inline std::unordered_map<std::string, int> missingCount;

    static inline bool initialized = false;

    static inline std::map<std::string, std::shared_ptr<AudioDevice>> idToDevices;

    static ma_result onDeviceInit(
        ma_device* pDevice, const ma_device_config* pConfig,
        ma_device_descriptor* pDescriptorPlayback, ma_device_descriptor* pDescriptorCapture
    ) {
        if (!initialized) return MA_SUCCESS;
        if (pDevice == nullptr) return MA_INVALID_ARGS;
        if (pDevice->pUserData == nullptr) return MA_SUCCESS;  // Likely an enumeration probe
        return refreshDevices();
    }

    static ma_result onDeviceUninit(ma_device* pDevice) {
        if (!initialized) return MA_SUCCESS;
        if (pDevice == nullptr) return MA_INVALID_ARGS;
        return refreshDevices(); // remove from list
    }

    static inline std::string defaultMicrophoneId = "";
    static inline std::string defaultSpeakerId = "";

    template <typename T, typename = std::enable_if_t<std::is_base_of<AudioDevice, T>::value>>
    static void handleDeviceLoop(
        const ma_device_info& deviceInfo,
        ma_device_type deviceType,
        const std::string& normalizedDeviceId,
        ma_format format,
        ma_uint32 sampleRate,
        ma_uint32 channels,
        T*& defaultDevice,
        std::function<std::shared_ptr<T>()> creationCallback
    ) {
        T* device = dynamic_cast<T*>(getDeviceById(normalizedDeviceId));

        if (device == nullptr) {
            auto newDevice = creationCallback();
            if (newDevice == nullptr) return;

            device = newDevice.get();
            addDevice(std::move(newDevice));
        }

        device->updateDevice(deviceInfo, deviceType, format, sampleRate, channels);

        if (device->isDefault)
            defaultDevice = device;
    }

public:
    /// <summary>
    /// Initializes SoundIO and devices.
    /// </summary>
    /// <returns>Initialization result</returns>
    static ma_result initialize();

    /// <summary>
    /// Shuts down SoundIO, releases devices and uninitializes contexts.
    /// </summary>
    /// <returns></returns>
    static ma_result shutdown() {
        if (!initialized) return MA_SUCCESS;

        idToDevices.clear();
        nodes.clear();
        missingCount.clear();

        ma_result result = ma_context_uninit(&context);

        defaultMicrophoneId.clear();
        defaultSpeakerId.clear();
        initialized = false;

        SI_LOG("SoundIO shutdown");

        return MA_SUCCESS;
    }

protected:
    template <typename T>
    static void addDevice(std::shared_ptr<T> device) {
        static_assert(std::is_base_of<AudioDevice, T>::value, "T must derive from AudioDevice");
        idToDevices.emplace(device->id, std::move(device));
    }

    static void removeDevice(const std::string& deviceId) {
        auto it = idToDevices.find(deviceId);
        if (it != idToDevices.end())
            idToDevices.erase(it); 
    }

public:
    /// <summary>
    /// Forces a device list refresh
    /// </summary>
    /// <returns>Refresh result</returns>
    static ma_result refreshDevices();

    /// <summary>
    /// Gets a device by its normalized id
    /// </summary>
    /// <param name="id">Normalized device id</param>
    /// <returns>Device if found, nullptr if not</returns>
    static AudioDevice* getDeviceById(const std::string& id) {
        auto it = idToDevices.find(id);
        return it != idToDevices.end() ? it->second.get() : nullptr;
    }

    /// <summary>
    /// Gets all current devices (input/output)
    /// </summary>
    /// <returns>A vector of the following devices</returns>
    static std::vector<AudioDevice*> getAllDevices() {
        std::vector<AudioDevice*> out;
        out.reserve(idToDevices.size());
        for (auto& kv : idToDevices)
            out.push_back(kv.second.get());
        return out;
    }

    /// <summary>
    /// Gets all current microphones (input devices)
    /// </summary>
    /// <returns>A vector of the following devices</returns>
    static std::vector<AudioMicrophoneDevice*> getAllMicrophones() {
        std::vector<AudioMicrophoneDevice*> out;
        out.reserve(idToDevices.size());
        for (auto& kv : idToDevices)
        if (auto* mic = dynamic_cast<AudioMicrophoneDevice*>(kv.second.get()))
            out.push_back(mic);
        return out;
    }

    /// <summary>
    /// Gets all current speakers (output devices)
    /// </summary>
    /// <returns>A vector of the following devices</returns>
    static std::vector<AudioSpeakerDevice*> getAllSpeakers() {
        std::vector<AudioSpeakerDevice*> out;
        out.reserve(idToDevices.size());
        for (auto& kv : idToDevices)
        if (auto* spk = dynamic_cast<AudioSpeakerDevice*>(kv.second.get())) 
            out.push_back(spk);
        return out;
    }

    /// <summary>
    /// Gets the default microphone/input device of this system
    /// </summary>
    /// <param name="autoWake">True by default, will wake up the device if found</param>
    /// <returns>Device if found else nullptr</returns>
    static AudioMicrophoneDevice* getDefaultMicrophone(bool autoWake = true) {
        if (defaultMicrophoneId.empty())
            return nullptr;

        auto device = getDeviceById(defaultMicrophoneId);
        if (!device) return nullptr;
        
        auto* microphone = dynamic_cast<AudioMicrophoneDevice*>(device);
        microphone->ensureAwake();
        return microphone;
    }

    /// <summary>
    /// Gets the default speaker/output device of this system
    /// </summary>
    /// <param name="autoWake">True by default, will wake up the device if found</param>
    /// <returns>Device if found else nullptr</returns
    static AudioSpeakerDevice* getDefaultSpeaker(bool autoWake = true) {
        if (defaultSpeakerId.empty())
            return nullptr;

        auto device = getDeviceById(defaultSpeakerId);
        if (!device) return nullptr;

        auto* speaker = dynamic_cast<AudioSpeakerDevice*>(device);
        speaker->ensureAwake();
        return speaker;
    }

protected:
    static inline std::vector<std::shared_ptr<AudioNode>> nodes;

    template <typename T, typename... Args>
    static T* registerNode(Args&&... args) {
        static_assert(std::is_base_of<AudioNode, T>::value,
            "T must derive from AudioNode");

        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        T* raw = ptr.get();
        nodes.push_back(std::move(ptr)); // nodes = std::vector<std::shared_ptr<AudioNode>>
        return raw;
    }

public:

    // creating inputs, outputs etc
    // input
    static AudioFileInput* createFileInput() {
        return registerNode<AudioFileInput>();
    }
    static AudioStreamInput* createStreamInput(const AudioFormat& format) {
        return registerNode<AudioStreamInput>(format);
    }

    // mixer
    /*static AudioCombiner* createCombiner(AudioInput* input = nullptr, AudioOutput* device = nullptr) {
        
    }
    */

    // output
    
    static AudioFileOutput* createFileOutput() {
        return registerNode<AudioFileOutput>();
    }
    static AudioStreamOutput* createStreamOutput(const AudioFormat& format) {
        return registerNode<AudioStreamOutput>(format);
    }

    // player
    static AudioPlayer* createAudioPlayer(AudioInput* input) {

    }

    static AudioFormat createAudioFormat(ma_format format, ma_uint32 channels, ma_uint32 sampleRate) {
        return AudioFormat(format, channels, sampleRate);
    }
};

inline ma_result SoundIO::initialize() {
    if (initialized) return MA_NO_MESSAGE;

    // initialize miniaudio context with default backends
    ma_result result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) return result;
    if (context.backend == ma_backend_null) return MA_BACKEND_NOT_ENABLED;

    // setup callbacks
    SI_LOG("SoundIO initialize: backend=" << context.backend);
    refreshDevices();

    initialized = true;
    return result;
}

inline ma_result SoundIO::refreshDevices() {
    ma_device_info* speakers;
    ma_uint32 speakerCount;
    ma_device_info* microphones;
    ma_uint32 microphoneCount;

    SI_LOG("refreshDevices() called from:");

    ma_context_get_devices(&context, &speakers, &speakerCount, &microphones, &microphoneCount);
    SI_LOG("refreshDevices: speakers=" << speakerCount << " mics=" << microphoneCount);

    ma_result result = MA_SUCCESS;

    AudioMicrophoneDevice* defaultMicrophone = nullptr;
    AudioSpeakerDevice* defaultSpeaker = nullptr;

    // track all IDs we encounter this refresh
    std::unordered_set<std::string> seenIds;

    loopDevices(context, speakers, speakerCount, ma_device_type_playback,
        [&result, &defaultSpeaker, &seenIds]
        (ma_device_info deviceInfo, std::string normalizedDeviceId, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
            seenIds.insert(normalizedDeviceId);

            auto creationCallback = [&normalizedDeviceId]() -> std::shared_ptr<AudioSpeakerDevice> {
                return std::make_shared<AudioSpeakerDevice>(normalizedDeviceId, &context);
            };

            handleDeviceLoop<AudioSpeakerDevice>(
                deviceInfo, ma_device_type_playback, normalizedDeviceId, format, sampleRate, channels,
                defaultSpeaker, creationCallback
            );
        }
    );

    if (result != MA_SUCCESS) return result;

    loopDevices(context, microphones, microphoneCount, ma_device_type_capture,
        [&result, &defaultMicrophone, &seenIds]
        (ma_device_info deviceInfo, std::string normalizedDeviceId, ma_format format, ma_uint32 sampleRate, ma_uint32 channels) {
            seenIds.insert(normalizedDeviceId);

            auto creationCallback = [&normalizedDeviceId]() -> std::shared_ptr<AudioMicrophoneDevice> {
                return std::make_shared<AudioMicrophoneDevice>(normalizedDeviceId, &context);
            };

            handleDeviceLoop<AudioMicrophoneDevice>(
                deviceInfo, ma_device_type_capture, normalizedDeviceId, format, sampleRate, channels,
                defaultMicrophone, creationCallback
            );
        }
    );

    if (result != MA_SUCCESS) return result;

    for (auto& kv : idToDevices) {
        if (seenIds.count(kv.first) == 0)
            missingCount[kv.first]++;
        else
            missingCount[kv.first] = 0;
    }

    for (auto it = idToDevices.begin(); it != idToDevices.end(); ) {
        if (missingCount[it->first] > 1) {
            missingCount.erase(it->first);
            it = idToDevices.erase(it);
        }
        else ++it;
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

    SI_LOG("defaults: speakerId=" << defaultSpeakerId << " micId=" << defaultMicrophoneId);
    return MA_SUCCESS;
}
