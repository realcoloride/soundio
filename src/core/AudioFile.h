#pragma once

#include "../core/AudioStream.h"

class AudioFile : public AudioStream {
protected:
    ma_decoder decoder;
    ma_encoder encoder;
    bool hasDecoder = false;
    bool hasEncoder = false;

    AudioFile(bool isSource, bool isSink) : AudioStream(AudioFormat{}, isSource, isSink) {}

    ma_result openDecoder(const std::string& filePath) {
        if (hasDecoder) {
            ma_decoder_uninit(&decoder);
            hasDecoder = false;
        }

        ma_result result = ma_decoder_init_file(filePath.c_str(), nullptr, &decoder);
        if (result == MA_SUCCESS) {
            hasDecoder = true;
            audioFormat.sampleRate = decoder.outputSampleRate;
            audioFormat.channels = decoder.outputChannels;
            audioFormat.format = decoder.outputFormat;
            renegotiate();
        }
        return result;
    }

    ma_result openEncoder(const std::string& filePath, const AudioFormat& targetFormat) {
        if (hasEncoder) {
            ma_encoder_uninit(&encoder);
            hasEncoder = false;
        }

        ma_encoder_config config = ma_encoder_config_init(
            ma_encoding_format_wav,
            targetFormat.toMaFormat(),
            targetFormat.channels,
            targetFormat.sampleRate
        );

        ma_result result = ma_encoder_init_file(filePath.c_str(), &config, &encoder);
        if (result == MA_SUCCESS) {
            hasEncoder = true;
            audioFormat = targetFormat;
            renegotiate();
        }
        return result;
    }

    void closeDecoder() {
        if (hasDecoder) {
            ma_decoder_uninit(&decoder);
            hasDecoder = false;
        }
    }

    void closeEncoder() {
        if (hasEncoder) {
            ma_encoder_uninit(&encoder);
            hasEncoder = false;
        }
    }

    ma_result readFromFile(void* pOut, ma_uint32 frameCount, ma_uint32* pFramesRead) {
        if (!hasDecoder) return MA_DEVICE_NOT_INITIALIZED;
        ma_uint64 framesRead;
        ma_result result = ma_decoder_read_pcm_frames(&decoder, pOut, frameCount, &framesRead);
        if (pFramesRead) *pFramesRead = (ma_uint32)framesRead;
        return result;
    }

    ma_result writeToFile(const void* pData, ma_uint32 frameCount) {
        if (!hasEncoder) return MA_DEVICE_NOT_INITIALIZED;
        ma_uint64 framesWritten;
        return ma_encoder_write_pcm_frames(&encoder, pData, frameCount, &framesWritten);
    }

public:
    virtual ~AudioFile() {
        closeDecoder();
        closeEncoder();
    }
};