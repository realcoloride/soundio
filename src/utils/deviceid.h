#pragma once
#include "../include.h"
#include "./wcharconvert.h"

constexpr size_t MAX_DEVICE_ID_BUFFER = 512;

static bool normalizeDeviceId(ma_backend backend, const ma_device_id& deviceId, ma_device_type deviceType, std::string* betterId) {
    if (!betterId) return false; // null pointer check

    std::ostringstream ss;
    std::string deviceIdStr;

    // extract device ID based on backend type
    switch (backend) {
        case ma_backend_wasapi: {
            deviceIdStr = convertWideCharToString(deviceId.wasapi, MAX_DEVICE_ID_BUFFER);
            break;
        }

        case ma_backend_dsound: {
            std::ostringstream guidStream;
            for (int i = 0; i < 16; i++) {
                guidStream << std::hex << std::setw(2) << std::setfill('0') << (int)deviceId.dsound[i];
            }
            deviceIdStr = guidStream.str();
            break;
        }

        case ma_backend_winmm:
            deviceIdStr = std::to_string(deviceId.winmm);
            break;

        case ma_backend_alsa:
            deviceIdStr = deviceId.alsa;
            break;

        case ma_backend_pulseaudio:
            deviceIdStr = deviceId.pulse;
            break;

        case ma_backend_sndio:
            deviceIdStr = deviceId.sndio;
            break;

        case ma_backend_audio4:
            deviceIdStr = deviceId.audio4;
            break;

        case ma_backend_coreaudio:
            deviceIdStr = deviceId.coreaudio;
            break;

        case ma_backend_oss:
            deviceIdStr = deviceId.oss;
            break;

        case ma_backend_webaudio:
            deviceIdStr = deviceId.webaudio;
            break;

        case ma_backend_aaudio:
            deviceIdStr = std::to_string(deviceId.aaudio);
            break;

        case ma_backend_jack:
            deviceIdStr = std::to_string(deviceId.jack);
            break;

        case ma_backend_null:
            deviceIdStr = std::to_string(deviceId.nullbackend);
            break;

        case ma_backend_opensl:
            deviceIdStr = std::to_string(deviceId.opensl);
            break;

        case ma_backend_custom: {
            std::ostringstream customStream;
            if (deviceId.custom.i != 0) {
                customStream << deviceId.custom.i;
            }
            if (deviceId.custom.s[0] != '\0') {
                if (!customStream.str().empty()) customStream << " | ";
                customStream << deviceId.custom.s;
            }
            if (deviceId.custom.p != nullptr) {
                if (!customStream.str().empty()) customStream << " | ";
                customStream << deviceId.custom.p;
            }
            deviceIdStr = customStream.str();
            break;
        }

        default:
            return false;
    }

    // ensure id is valid
    if (deviceIdStr.empty()) return false;

    // final string: "<DeviceID>_<DeviceType>"
    ss << deviceIdStr << "_" << deviceType;

    *betterId = ss.str();
    return true;
}