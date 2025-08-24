# SoundIO

> [!WARNING]
> 🔨 Work in progress

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
[![License](https://img.shields.io/badge/MIT-green?style=for-the-badge
)](LICENSE) 

**SoundIO** (**Sound** **I**nput/**O**utput) is a header-only, cross-platform, high-level audio library for C++ built on top of miniaudio.

It gives you a **node-based** audio graph abstraction so you can connect audio inputs, outputs, files and manage playback without worrying about low-level device handling, memory safety, or backend quirks for those who want to work with audio quickly and easily.

Works anywhere miniaudio works - from desktop to mobile to the web;

***Because simple audio shouldn't be such a headache.***

# Demo

Not yet :v

# Features

- **Header-only**, modern `C++17+`, no extra library required
- **Extremely simple API**, node-based graph, subscription based calls
- **Low memory footprint and low overhead**, simply wrapping miniaudio
- **Format negociation built-in**, no need to resample or convert between inputs and outputs
- **Supports for input and output devices**, microphones and speakers, for all platforms are built in, with ability to fetch them with a normalized id for all platforms
- **File playback and recording**, (WAV, MP3, OGG, PCM) - support for the following formats are out of the box
- **Mixing, combining, resampling, playback**, built-in for complex but easy-to-use audio manipulation
- **Runs anywhere miniaudio runs**: Windows, macOS, Linux, Android, iOS, Web _(see supported backends below)_

# Table of contents
- [SoundIO](#soundio)
- [Demo](#demo)
- [Features](#features)
- [Table of contents](#table-of-contents)
    - [Remaining features](#remaining-features)
      - [File Features](#file-features)
    - [Mixing Features](#mixing-features)
    - [Playback](#playback)
    - [Other](#other)
- [Specifications](#specifications)
- [Installation](#installation)
- [Usage](#usage)
- [Documentation](#documentation)
- [Building](#building)
- [Disclaimer](#disclaimer)

### Remaining features
<details> <summary>Click to see the remaining features...</summary>

#### File Features
* `AudioFileOutput.h` - Exports file data

### Mixing Features
* `AudioMixer.h` - Base class for mixing PCM audio
* `AudioCombiner.h` - Combines multiple inputs into a single mixed output
* `AudioResampler.h` - Resamples an input into 

### Playback
* `AudioPlayer.h` - Manages the playback of an input into the output

### Other

Shutting down, finishing the SoundIO.h class, cleaning up code, documenting methods,
QA + Testing and example scripts should be written.

</details>

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

> [!WARNING]
> To create SoundIO instances, it is **required** that you use `SoundIO::create*` because their memory management will be handled by SoundIO.

Playing a file

```cpp
// get default speaker
auto* speaker = SoundIO::getDefaultSpeaker();

// create a file input, and open the file
auto* file = SoundIO::createFileInput();
ma_result result = file->open("sample.mp3");

// if the file was successfully loaded
if (result == MA_SUCCESS) 
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
if (result == MA_SUCCESS) 
    // the output of the microphone will be saved to the file automatically.
    microphone->subscribe(file);
```

More examples are available [here](https://github.com/realcoloride/soundio/tree/main/examples/).

# Documentation

<details> <summary>Click here to see the full documentation</summary>



</details>


# Building



# Disclaimer

🚀 If you have an issue or idea, let me know in the [**Issues**](https://github.com/realcoloride/soundio/issues) section.

📜 If you use this library, you also bound by the terms of [`miniaudio.h`'s license](https://github.com/aws/mit-0) and this library's [MIT license](LICENSE).

☕ **Want to support me?** You can send me a coffee on ko.fi: https://ko-fi.com/coloride.

© *(real)Coloride - 2025, Licensed MIT.*