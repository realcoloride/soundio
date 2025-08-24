# SoundIO

> [!WARNING]
> 🔨 Work in progress

![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
[![License](https://img.shields.io/badge/MIT-green?style=for-the-badge
)](LICENSE) 

**SoundIO** (**Sound** **I**nput/**O**utput) is a header-only, cross-platform, high-level audio library for C++ built on top of [miniaudio](https://miniaud.io).

It gives you a **node-based** audio graph abstraction so you can connect audio inputs, outputs, files and manage playback without worrying about low-level device handling, memory safety, or backend quirks for those who want to work with audio quickly and easily.

Works anywhere [miniaudio](https://miniaud.io) works - from desktop to mobile to the web;

***Because simple audio shouldn't be such a headache.***

# Demo

Not yet :v

# Features

- **Header-only**, modern `C++17+`, no extra library required
- **Extremely simple API**, node-based graph, subscription based calls
- **Low memory footprint and low overhead**, simply wrapping [miniaudio](https://miniaud.io)
- **Format negociation built-in**, no need to resample or convert between inputs and outputs
- **Supports for input and output devices**, microphones and speakers, for all platforms are built in, with ability to fetch them with a normalized id for all platforms
- **File playback and recording**, (WAV, MP3, OGG, PCM) - support for the following formats are out of the box
- **Mixing, combining, resampling, playback**, built-in for complex but easy-to-use audio manipulation
- **Runs anywhere [miniaudio](https://miniaud.io) runs**: Windows, macOS, Linux, Android, iOS, Web _(see supported backends below)_

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
    - [Debugging](#debugging)
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

Shutting down, finishing the SoundIO.h class, cleaning up code, documenting methods, detecting properly when devices uninit (miniaudio is not properly handling this well), making debugging optional
Better building script
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

*__TL;DR:__ if its supported by [miniaudio](https://miniaud.io), it is supported by SoundIO.*

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

<details><summary>Playing a file</summary>

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
</details>

<details><summary>Playing a file with playback</summary>

```cpp

```
</details>

> [!IMPORTANT]
> To avoid wasting resources, devices are **not active by default**.  
> You **must** wake up a device with `device->ensureAwake()` before using it.  
> The default microphone and speaker are automatically woken up when you request them (this is optional, but enabled by default).

<details><summary>Listing devices and getting their information</summary>

```cpp
for (auto* device : SoundIO::getAllDevices()) {
    // Device name / normalized id
    std::cout << "Device: " << device->name << " (ID: " << device->id << ")\n";

    // Device type
    std::cout << "  Type: " << (device->deviceType == ma_device_type_playback ? "Speaker" : "Microphone") << "\n";

    // Device channels
    std::cout << "  Channels: " << device->deviceFormat.channels << "\n";

    // Device sample rate
    std::cout << "  Sample Rate: " << device->deviceFormat.sampleRate << "\n";

    // Device format
    std::cout << "  Format: " << device->deviceFormat.format << "\n";

    // Is device default
    if (device->isDefault) std::cout << "  [default]" << "\n";
}
```
</details>

<details><summary>Simple microphone to speaker loopback</summary>

```cpp
// get default microphone and speaker
auto* microphone = SoundIO::getDefaultMicrophone();
auto* speaker = SoundIO::getDefaultSpeaker();

// initialize loopback.
// resampling and format negociation is automatically handled by SoundIO.
microphone->subscribe(speaker);
```
</details>

<details><summary>Recording microphone data to a file</summary>

```cpp
// get default microphone
auto* microphone = SoundIO::getDefaultMicrophone();

// create a file input, and open the file
// format IS required (mp3, wav, pcm etc)
auto* file = SoundIO::createFileOutput();
ma_result result = file->open("recording.wav", mic->deviceFormat);

// if the file was successfully loaded
if (result == MA_SUCCESS) 
    // the output of the microphone will be saved to the file automatically.
    microphone->subscribe(file);
```
</details>

_More detailed examples are available [here](https://github.com/realcoloride/soundio/tree/main/examples/)._


### Debugging

By default, SoundIO will remain silent and not print anything out because most methods return an `ma_result`, however using the following define statement will enable verbose debugging statements.

```cpp
#define SOUNDIO_LOG_ENABLED 1
```

# Documentation

<details> <summary>Click here to see the full documentation</summary>



</details>


# Building



# Disclaimer

🚀 If you have an issue or idea, let me know in the [**Issues**](https://github.com/realcoloride/soundio/issues) section.

📜 If you use this library, you also bound by the terms of [`miniaudio.h`'s license](https://github.com/aws/mit-0) and this library's [MIT license](LICENSE).

☕ **Want to support me?** You can send me a coffee on ko.fi: https://ko-fi.com/coloride.

© *(real)Coloride - 2025, Licensed MIT.*