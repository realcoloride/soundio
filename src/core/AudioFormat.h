#pragma once

#include "../include.h"

struct AudioFormat {
    ma_encoding_format encodingFormat = ma_encoding_format_unknown;
    ma_format  format = ma_format_f32;
    ma_uint32  channels = 2;
    ma_uint32  sampleRate = 48000;

    AudioFormat(ma_format format, ma_uint32 channels, ma_uint32 sampleRate) {
        this->format = format;
        this->channels = channels;
        this->sampleRate = sampleRate;
    }

    AudioFormat() : format(ma_format_unknown), channels(0), sampleRate(0) {}

    ma_format toMaFormat() const { return format; }
    ma_uint32 frameSizeInBytes(ma_uint32 frames = 1) const {
        return frames * (ma_uint32)(channels * ma_get_bytes_per_sample(format));
    }

    static AudioFormat Stereo48kF32() { return { ma_format_f32, 2, 48000 }; }

    bool operator==(const AudioFormat& o) const {
        return format == o.format && channels == o.channels && sampleRate == o.sampleRate;
    }
    bool operator!=(const AudioFormat& o) const { return !(*this == o); }
};
