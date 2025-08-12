# SoundIO

> [!WARNING]
> 🔨 Work in progress

# Intro

**SoundIO** (**Sound** **I**nput **O**utput) is a simple agnostic cross-platform header only high level node based audio graph library written in C++ using `miniaudio.h`.

It wraps the miniaudio api into a node based graph high level easy with classes to use audio library without the hassle of handling low level or device management and memory safety.

# Features

### Remaining features

#### File Features
* `AudioFile.h` - Base class for handling files
* `AudioFileInput.h` - Imports file data
* `AudioFileOutput.h` - Exports file data

### Mixing Features
* `AudioMixer.h` - Base class for mixing PCM audio
* `AudioCombiner.h` - Combines multiple inputs into a single mixed output
* `AudioResampler.h` - Resamples an input into 

### Playback
* `AudioPlayer.h` - Manages the playback of an input into the output

Shutting down, finishing the SoundIO.h class, cleaning up code, documenting methods,
QA + Testing and example scripts should be written.

# Specifications

* **C++ Standard:** C++17 or above, exclusively written in std code only
* **Supported audio formats**: PCM (all formats), WAV, MP3, OGG
* **Supported audio backends**:

|Operating System|Supported backends|
|-|-|
|**Windows**|WASAPI, DirectSound, WinMM|
|**macOS**/**iOS**|Core Audio|
|**Linux**|ALSA, PulseAudio, JACK|
|**FreeBSD**|OSS, sndio, audio(4)|
|**OpenBSD**|OSS, sndio, audio(4)|
|**NetBSD**|OSS, sndio, audio(4)|
|**Android**|AAudio, OpenSL/ES|
|**Web**|Emscripten, WebAudio|

*__TL;DR:__ if its supported by miniaudio.h, it is supported by SoundIO.*

# Installation


# Usage

To use SoundIO, simply include the header file.
```cpp
#include <SoundIO.h>
```

Initialize and shutting down SoundIO:
```cpp
// storing result is optional.
ma_result result = SoundIO::initialize();

// shutting down (when done to clean up)
ma_result result = SoundIO::shutdown();
```

Playing a file

```cpp
// get default speaker
auto* speaker = SoundIO::getDefaultSpeaker();

// create a file input, and open the file
auto* file = SoundIO::createFileInput();
ma_result result = file->open("sample.mp3");

// if the file was successfully loaded
if (ma_result == MA_SUCCESS) 
    // the output of the file will be drained to the speaker.
    file->subscribe(speaker);

```

Playing a file with playback
```cpp

```

Listing devices and getting their information
```cpp
// by default, when getDefaultSpeaker or Microphone have an argument called autoWake defaulting to true that wakes up devices.
// in order to avoid wasting resources, only wake up the devices using ensureAwake and sleep when you do not need them anymore.

```

Simple microphone to speaker loopback
```cpp
// get default microphone and speaker
auto* microphone = SoundIO::getDefaultMicrophone();
auto* speaker = SoundIO::getDefaultSpeaker();

// initialize loopback.
// resampling and format negociation is automatically handled by SoundIO.
microphone->subscribe(speaker);
```

Recording microphone data to a file
```cpp
// get default microphone
auto* microphone = SoundIO::getDefaultMicrophone();

// create a file input, and open the file
// format IS required (mp3, wav, pcm etc)
// for the sake of simplicity, we take the mic's default PCM format (.pcm).
auto* file = SoundIO::createFileOutput();
ma_result result = file->open("recording.pcm", mic->deviceFormat);

// if the file was successfully loaded
if (ma_result == MA_SUCCESS) 
    // the output of the microphone will be saved to the file automatically.
    microphone->subscribe(file);
```

More examples are available [here](https://github.com/realcoloride/soundio/tree/main/examples/).

# Building



# Disclaimer