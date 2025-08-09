#pragma once
#include "../include.h"
#include "./wcharconvert.h"

constexpr size_t MAX_DEVICE_ID_BUFFER = 512;

// utils
static inline void to_lower_in_place(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
}

static inline void trim_in_place(std::string& s) {
    auto not_space = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_space));
    s.erase(std::find_if(s.rbegin(), s.rend(), not_space).base(), s.end());
}

static inline void collapse_ws_in_place(std::string& s) {
    std::string out; out.reserve(s.size());
    bool in_ws = false;
    for (unsigned char c : s) {
        if (std::isspace(c)) { in_ws = true; continue; }
        if (in_ws && !out.empty()) out.push_back(' ');
        out.push_back((char)c);
        in_ws = false;
    }
    s.swap(out);
}

static inline void strip_nonprintable_in_place(std::string& s) {
    s.erase(std::remove_if(s.begin(), s.end(),
        [](unsigned char c) { return !std::isprint(c); }), s.end());
}

static bool normalizeDeviceId(ma_backend backend,
    const ma_device_id& deviceId,
    ma_device_type deviceType,
    std::string* betterId
) {
    if (!betterId) return false;

    std::string deviceIdStr;

    switch (backend) {
        case ma_backend_wasapi: {
            deviceIdStr = convertWideCharToString(deviceId.wasapi, MAX_DEVICE_ID_BUFFER);
        } break;

        case ma_backend_dsound: {
            std::ostringstream guidStream;
            guidStream << std::nouppercase << std::hex;
            for (int i = 0; i < 16; ++i) {
                guidStream << std::setw(2) << std::setfill('0') << (int)deviceId.dsound[i];
            }
            deviceIdStr = guidStream.str(); // already lowercase
        } break;

        case ma_backend_winmm:      deviceIdStr = std::to_string(deviceId.winmm); break;
        case ma_backend_alsa:       if (deviceId.alsa)      deviceIdStr = deviceId.alsa;      break;
        case ma_backend_pulseaudio: if (deviceId.pulse)     deviceIdStr = deviceId.pulse;     break;
        case ma_backend_sndio:      if (deviceId.sndio)     deviceIdStr = deviceId.sndio;     break;
        case ma_backend_audio4:     if (deviceId.audio4)    deviceIdStr = deviceId.audio4;    break;
        case ma_backend_coreaudio:  if (deviceId.coreaudio) deviceIdStr = deviceId.coreaudio; break;
        case ma_backend_oss:        if (deviceId.oss)       deviceIdStr = deviceId.oss;       break;
        case ma_backend_webaudio:   if (deviceId.webaudio)  deviceIdStr = deviceId.webaudio;  break;
        case ma_backend_aaudio:     deviceIdStr = std::to_string(deviceId.aaudio); break;
        case ma_backend_jack:       deviceIdStr = std::to_string(deviceId.jack); break;
        case ma_backend_null:       deviceIdStr = std::to_string(deviceId.nullbackend); break;
        case ma_backend_opensl:     deviceIdStr = std::to_string(deviceId.opensl); break;
        case ma_backend_custom: {
            std::ostringstream customStream;
            if (deviceId.custom.i != 0) customStream << deviceId.custom.i;
            if (deviceId.custom.s[0] != '\0') {
                if (customStream.tellp() > 0) customStream << " | ";
                customStream << deviceId.custom.s;
            }
            if (deviceId.custom.p != nullptr) {
                if (customStream.tellp() > 0) customStream << " | ";
                customStream << deviceId.custom.p;
            }
            deviceIdStr = customStream.str();
            break;
        } 
        default: return false;
    }

    if (deviceIdStr.empty()) return false;

    strip_nonprintable_in_place(deviceIdStr);
    trim_in_place(deviceIdStr);
    collapse_ws_in_place(deviceIdStr);
    to_lower_in_place(deviceIdStr);

    // backend tag to avoid rare cross-backend collisions
    auto backendTag = [&]() -> const char* {
        switch (backend) {
            case ma_backend_wasapi: return "wasapi";
            case ma_backend_dsound: return "dsound";
            case ma_backend_winmm: return "winmm";
            case ma_backend_alsa: return "alsa";
            case ma_backend_pulseaudio: return "pulse";
            case ma_backend_sndio: return "sndio";
            case ma_backend_audio4: return "audio4";
            case ma_backend_coreaudio: return "coreaudio";
            case ma_backend_oss: return "oss";
            case ma_backend_webaudio: return "webaudio";
            case ma_backend_aaudio: return "aaudio";
            case ma_backend_jack: return "jack";
            case ma_backend_opensl: return "opensl";
            case ma_backend_null: return "null";
            case ma_backend_custom: return "custom";
            default: return "unknown";
        }
    } ();

    // final key: "<backend>:<sanitized-id>_<deviceType>"
    std::ostringstream ss;
    ss << backendTag << ":" << deviceIdStr << "_" << (int)deviceType;

    *betterId = ss.str();
    return true;
}
