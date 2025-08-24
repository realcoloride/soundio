#include <SoundIO.h>
#include <cmath>
#include <vector>
#include <thread>
#include <iostream>
#define M_PI 3.14159265358979323846f

// audio parameters
const float duration = 10.0f; // duration of the sine wave
const float freq = 440.0f;    // frequency in Hz (A4)
const float amplitude = 0.2f; // volume (0.0 to 1.0)

// sine wave example
int main() {
    std::cout << "[SoundIO] sine wave example" << std::endl;
    
    std::cout << "initializing soundio..." << std::endl;
    // init SoundIO
    SoundIO::initialize();

    // get speaker & format
    // this assumes you have atleast one speaker
    auto* spk = SoundIO::getDefaultSpeaker();

    // the following example is format agnostic;
    // but i prefer still taking the device's native format
    AudioFormat format = spk->deviceFormat;
    
    // create stream & subscribe
    auto* stream = SoundIO::createStreamInput(format);
    stream->subscribe(spk);
    
    // phase accumulator for sine wave generation
    float phase = 0.0f;
    const float twoPiF = 2.0f * M_PI * freq / format.sampleRate;

    // frames to process
    ma_uint32 batchFrames = stream->getOutputRingFrames();
    int totalFrames = duration * format.sampleRate;
    int framesLeft = totalFrames;

    std::cout << "starting generation..." << std::endl;
    std::cout << "sine wave is playing... (" << duration << "s)" << std::endl;
    
    // main generation loop
    while (framesLeft > 0) {
        // calculate current batch size (remaining frames or max batch size)
        ma_uint32 currentBatch = min(batchFrames, (ma_uint32)framesLeft);
        
        // wait until there's enough space in the audio buffer
        while (stream->getAvailableWriteFrames() < currentBatch)
            std::this_thread::yield();

        // generate audio samples for this batch
        std::vector<float> buffer(currentBatch * format.channels);

        for (ma_uint32 j = 0; j < currentBatch; ++j) {
            // generate sin wave sample
            float sample = amplitude * std::sin(phase);
            phase += twoPiF;
            
            // wrap phase to avoid floating point precision issues
            if (phase > 2 * M_PI) phase -= 2 * M_PI;
            
            // copy sample to all audio channels (mono -> stereo/multichannel)
            for (ma_uint32 ch = 0; ch < format.channels; ++ch)
                buffer[j * format.channels + ch] = sample;
        }
        
        // submit audio data to the stream
        stream->submitPCM(buffer.data(), currentBatch);
        framesLeft -= currentBatch;
    }

    // wait for the sine wave to be processed
    long time = format.framesToMs(batchFrames);
    std::this_thread::sleep_for(std::chrono::milliseconds(time));

    std::cout << "sine wave ended" << std::endl;
    std::cout << "shutting down..." << std::endl;

    // cleanup
    SoundIO::shutdown();
    return 0;
}
