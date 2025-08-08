#include <shared/SoundIO.h>

ma_result SoundIO::initialize() {
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

ma_result SoundIO::refreshDevices() {
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