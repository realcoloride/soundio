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
    static ma_result initialize();
    static ma_result shutdown() {
        if (!initialized) return MA_SUCCESS;

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

    static AudioMicrophoneDevice* getDefaultMicrophone(bool autoWake = true) {
        if (defaultMicrophoneId.empty())
            return nullptr;

        auto* microphone = dynamic_cast<AudioMicrophoneDevice*>(getDeviceById(defaultMicrophoneId));
        microphone->ensureAwake();
        return microphone;
    }

    static AudioSpeakerDevice* getDefaultSpeaker(bool autoWake = true) {
        if (defaultSpeakerId.empty())
            return nullptr;

        auto* speaker = dynamic_cast<AudioSpeakerDevice*>(getDeviceById(defaultSpeakerId));
        speaker->ensureAwake();
        return speaker;
    }

protected:
    static inline std::vector<std::shared_ptr<AudioNode>> nodes;

    // shared_ptr cleanup happens automatically
    static void clearAllNodes() { nodes.clear(); }

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
    /*static AudioFileInput* createFileInput(const std::string& path) {
        auto* input = registerNode<AudioFileInput>();
        // input->open(path);
        return input;
    }
    */
    static AudioStreamInput* createStreamInput(const AudioFormat& format) {
        return registerNode<AudioStreamInput>(format);
    }

    // mixer
    /*static AudioCombiner* createCombiner(AudioInput* input = nullptr, AudioOutput* device = nullptr) {
        
    }
    static AudioResampler* createResampler(AudioFormat outputFormat, AudioInput* input = nullptr) {

    }
    static AudioPipe* createPipe() {

    }*/

    // output
    /*
    static AudioFileOutput* createFileOutput() {

    }
    */
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
    // Add a simple stack trace or debug info
    #ifdef _WIN32
        void* stack[10];
        unsigned short frames = CaptureStackBackTrace(0, 10, stack, NULL);
        for (unsigned short i = 0; i < frames; i++) {
            SI_LOG("  Frame " << i << ": " << stack[i]);
        }
    #endif


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
        if (missingCount[it->first] > 2) { // Only delete after 3 consecutive misses
            missingCount.erase(it->first);
            it = idToDevices.erase(it);
        }
        else {
            ++it;
        }
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
