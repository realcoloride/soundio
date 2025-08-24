#pragma once

#include "../core/AudioStream.h"
#include "../include.h"

class AudioFile : public virtual AudioEndpoint {
protected:
    ma_decoder decoder;
    ma_encoder encoder;
    bool hasDecoder = false;
    bool hasEncoder = false;
    std::string filePath;

public:
    ma_result bufferStatus = MA_SUCCESS;

    AudioFile(bool isSource, bool isSink) : AudioEndpoint() {
        bufferStatus = MA_SUCCESS;

        audioFormat = AudioFormat();
        canFillInputRing = isSink;
        canDrainOutputRing = isSource;
    }
    
    static ma_result checkDecode(const std::string& path) {
        ma_decoder temporaryDecoder;
        ma_result result = ma_decoder_init_file(path.c_str(), NULL, &temporaryDecoder);
        ma_decoder_uninit(&temporaryDecoder);
        return result;
    }

    ma_result checkEncode(const std::string& path) const {
        ma_encoder temporaryEncoder;
        ma_encoder_config encoderConfig = ma_encoder_config_init(
            audioFormat.encodingFormat, 
            audioFormat.format, 
            audioFormat.channels, 
            audioFormat.sampleRate
        );

        ma_result result = ma_encoder_init_file(path.c_str(), NULL, &temporaryEncoder);
        ma_encoder_uninit(&temporaryEncoder);
        return result;
    }

    ma_result prepareOpen(const std::string& path, ma_result(*checkMethod)(const std::string&)) {
        ma_result result = checkMethod(path);
        
        if (result == MA_SUCCESS) {
            filePath = path;
        }

        return result;
    }

    ma_result openDecoder() {
        closeDecoder();
        bufferStatus = MA_SUCCESS;

        ma_decoder_config* config = NULL;
        if (this->isInputSubscribed()) {
            auto* inputFormat = this->getInputFormat();
            ma_decoder_config decoderConfig = ma_decoder_config_init(inputFormat->format, inputFormat->channels, inputFormat->sampleRate);
            config = &decoderConfig;
        }

        ma_result result = ma_decoder_init_file(filePath.c_str(), NULL, &decoder);
        if (result == MA_SUCCESS) {
            hasDecoder = true;

            audioFormat.format = decoder.outputFormat;
            audioFormat.channels = decoder.outputChannels;
            audioFormat.sampleRate = decoder.outputSampleRate;
        }

        bufferStatus = result;
        return result;
    }

    ma_result openEncoder(const std::string& path, const AudioFormat& targetFormat) {
        closeEncoder();
        bufferStatus = MA_SUCCESS;

        audioFormat = targetFormat;

        ma_encoder_config config = ma_encoder_config_init(
            audioFormat.encodingFormat,
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
    const std::string& getFilePath() const { return filePath; }
    ma_result getBufferStatus() const { return bufferStatus; }
    const AudioFormat& getFormat() const { return audioFormat; }

    virtual ~AudioFile() {
        closeDecoder();
        closeEncoder();
    }
};