#pragma once
#include "../include.h"
#include "./deviceid.h"

static void loopDevices(
    ma_context& context, ma_device_info* devices, ma_uint32 deviceCount, ma_device_type deviceType,
    const std::function<void(ma_device_info, std::string, ma_format, ma_uint32, ma_uint32)>& callback
) {
    for (ma_uint32 i = 0; i < deviceCount; i++) {
        auto& deviceInfo = devices[i];

        // get detailed device info
        ma_device_info detailedInfo;
        if (ma_context_get_device_info(&context, ma_device_type_playback, &deviceInfo.id, &detailedInfo) != MA_SUCCESS)
            continue;

        // devices with no available formats are skipped
        ma_uint32 formatCount = detailedInfo.nativeDataFormatCount;
        if (formatCount == 0) continue;

        // the best format will be picked
        ma_format bestFormat = ma_format_unknown;
        ma_uint32 bestSampleRate = 44100;
        ma_uint32 bestChannels = 2;
        bool bestIsExclusive = false;

        for (ma_uint32 j = 0; j < formatCount; j++) {
            auto& deviceFormat = detailedInfo.nativeDataFormats[j];

            // check if this format supports Exclusive Mode
            bool isExclusive = (deviceFormat.flags & MA_DATA_FORMAT_FLAG_EXCLUSIVE_MODE) != 0;

            // skip formats that are completely unusable
            if (deviceFormat.format == ma_format_unknown || deviceFormat.sampleRate == 0)
                continue;

            // Prioritize:
            // Float32 > 16-bit
            // Higher sample rate
            // Exclusive mode preferred
            bool isBetter = false;

            // First valid format -> Always accept
            if (bestFormat == ma_format_unknown)
                isBetter = true;

            // Prefer Float32 over anything else
            else if (deviceFormat.format == ma_format_f32 && bestFormat != ma_format_f32)
                isBetter = true;

            // Prefer higher sample rates
            else if (deviceFormat.format == bestFormat && deviceFormat.sampleRate > bestSampleRate)
                isBetter = true;

            // Prefer more channels if everything else is equal
            else if (deviceFormat.format == bestFormat && deviceFormat.sampleRate == bestSampleRate &&
                deviceFormat.channels > bestChannels)
                isBetter = true;

            // Prefer Exclusive Mode if format/sample rate/channels are equal
            else if (deviceFormat.format == bestFormat && deviceFormat.sampleRate == bestSampleRate &&
                deviceFormat.channels == bestChannels && isExclusive && !bestIsExclusive)
                isBetter = true;

            if (!isBetter) continue;

            bestFormat = deviceFormat.format;
            bestSampleRate = deviceFormat.sampleRate;
            bestChannels = deviceFormat.channels;
            bestIsExclusive = isExclusive;
        }

        // Probable error or something I do not wanna deal with, skip
        std::string id;
        if (bestFormat == ma_format_unknown || 
            !normalizeDeviceId(context.backend, detailedInfo.id, deviceType, &id)) continue;

        callback(detailedInfo, id, bestFormat, bestSampleRate, bestChannels);
    }
}