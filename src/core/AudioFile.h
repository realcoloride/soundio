#pragma once

#include "../core/AudioStream.h"
#include "../include.h"

static const std::unordered_map<std::string, ma_encoding_format> extToFormat = {
    { ".mp3",  ma_encoding_format_mp3 },
    { ".wav",  ma_encoding_format_wav },
    { ".flac", ma_encoding_format_flac },
    { ".ogg",  ma_encoding_format_vorbis },
};

class AudioFile : public virtual AudioEndpoint {
protected:
    ma_decoder decoder;
    ma_encoder encoder;
    bool hasDecoder = false;
    bool hasEncoder = false;
    std::string filePath;
    
    bool isInputFinished;

    ma_encoding_format encodingFormat = ma_encoding_format_unknown;

public:
    ma_result bufferStatus = MA_SUCCESS;

    AudioFile(bool isSource, bool isSink) : AudioEndpoint(), isInputFinished(false) {
        bufferStatus = MA_SUCCESS;

        audioFormat = AudioFormat();
        canFillInputRing = isSink;
        canDrainOutputRing = isSource;
    }
    
protected:
    void setFormat(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_encoding_format encodingFormat = ma_encoding_format_unknown) {
        this->encodingFormat = encodingFormat;
    }

    ma_encoding_format guessEncodingFormat(std::filesystem::path path, ma_encoding_format specifiedFormat) const {
        if (specifiedFormat != ma_encoding_format_unknown)
            return specifiedFormat;

        std::string extensionString = path.extension().string();
        auto it = extToFormat.find(extensionString);
        if (it != extToFormat.end())
            return it->second;

        return ma_encoding_format_unknown;
    }

    ma_result openFileDecode(const std::string& path) {
        ma_decoder temporaryDecoder;
        ma_result result = ma_decoder_init_file(path.c_str(), NULL, &temporaryDecoder);

        if (result == MA_SUCCESS) {
            filePath = path;
            audioFormat = AudioFormat(
                temporaryDecoder.outputFormat,
                temporaryDecoder.outputChannels,
                temporaryDecoder.outputSampleRate
            );
            encodingFormat = ma_encoding_format_unknown;
        }

        ma_decoder_uninit(&temporaryDecoder);
        return result;
    }

    ma_result openFileEncode(
        const std::string& path,
        const AudioFormat& targetFormat,
        ma_encoding_format targetEncodingFormat = ma_encoding_format_unknown
    ) {
        ma_encoder temporaryEncoder;

        ma_encoder_config encoderConfig = ma_encoder_config_init(
            guessEncodingFormat(path, targetEncodingFormat),
            targetFormat.format,
            targetFormat.channels,
            targetFormat.sampleRate
        );

        ma_result result = ma_encoder_init_file(path.c_str(), &encoderConfig, &temporaryEncoder);
        if (result == MA_SUCCESS) {
            filePath = path;
            audioFormat = targetFormat;
            encodingFormat = encoderConfig.encodingFormat;
        }
        
        ma_encoder_uninit(&temporaryEncoder);
        return result;
    }

    ma_result openDecoder() {
        closeDecoder();
        bufferStatus = MA_SUCCESS;
        isInputFinished = false;

        if (!this->isOutputSubscribed()) return MA_NOT_CONNECTED;

        auto* outputFormat = this->getOutputFormat();
        ma_decoder_config config = ma_decoder_config_init(
            outputFormat->format, 
            outputFormat->channels, 
            outputFormat->sampleRate
        );

        ma_result result = ma_decoder_init_file(filePath.c_str(), &config, &decoder);
        if (result == MA_SUCCESS) {
            hasDecoder = true;

            audioFormat.format = decoder.outputFormat;
            audioFormat.channels = decoder.outputChannels;
            audioFormat.sampleRate = decoder.outputSampleRate;
        }

        bufferStatus = result;
        return result;
    }

    ma_result openEncoder(const std::string& path, const AudioFormat& targetFormat, ma_encoding_format encodingFormat) {
        closeEncoder();
        bufferStatus = MA_SUCCESS;
        audioFormat = targetFormat;

        ma_encoder_config config = ma_encoder_config_init(
            encodingFormat,
            audioFormat.format,
            audioFormat.channels,
            audioFormat.sampleRate
        );

        ma_result result = ma_encoder_init_file(path.c_str(), &config, &encoder);
        if (result == MA_SUCCESS) {
            hasEncoder = true;
            filePath = path;
            renegotiate(); 
        }

        bufferStatus = result;
        return result;
    }

    void closeDecoder() {
        if (hasDecoder) {
            ma_decoder_uninit(&decoder);
            hasDecoder = false;
            filePath.clear();
        }

        bufferStatus = MA_SUCCESS;
        isInputFinished = false;
    }

    void closeEncoder() {
        if (hasEncoder) {
            ma_encoder_uninit(&encoder);
            hasEncoder = false;
            filePath.clear();
        }
        bufferStatus = MA_SUCCESS;
    }

    ma_result readFromFile(void* pData, ma_uint32 frameCount, ma_uint32* framesRead) {
        if (!hasDecoder) {
            bufferStatus = MA_NO_DEVICE;
            return MA_NO_DEVICE;
        }

        ma_uint64 framesRead64;
        ma_result result = ma_decoder_read_pcm_frames(&decoder, pData, frameCount, &framesRead64);
        *framesRead = (ma_uint32)framesRead64;

        if (result == MA_SUCCESS && *framesRead == 0)
            isInputFinished = true;

        bufferStatus = result;
        return result;
    }

    ma_result writeToFile(const void* pData, ma_uint32 frameCount) {
        if (!hasEncoder) {
            bufferStatus = MA_NO_DEVICE;
            return MA_NO_DEVICE;
        }

        ma_result result = ma_encoder_write_pcm_frames(&encoder, pData, frameCount, nullptr);
        bufferStatus = result;
        return result;
    }

    bool isDecoderOpen() const { return hasDecoder; }
    bool isEncoderOpen() const { return hasEncoder; }
    
public:
    const std::string& getFilePath() const { return filePath; }
    ma_result getBufferStatus() const { return bufferStatus; }
    const AudioFormat& getFormat() const { return audioFormat; }

    virtual ~AudioFile() {
        closeDecoder();
        closeEncoder();
    }
};