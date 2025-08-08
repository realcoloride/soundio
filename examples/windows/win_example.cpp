#include <windows.h>
#include <iostream>
#include "../../src/SoundIO.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // 1) Init
    ma_result r = SoundIO::initialize();
    if (r != MA_SUCCESS) {
        MessageBoxW(NULL, L"SoundIO::initialize() failed", L"Error", MB_ICONERROR);
        return 1;
    }

    // 2) Use it
    if (auto* device = SoundIO::getDefaultPlaybackDevice()) {
        std::cout << device->name << std::endl; // use -mconsole so you can see this
    } else {
        MessageBoxW(NULL, L"No default playback device", L"Error", MB_ICONWARNING);
    }

    // 3) Shutdown (be nice)
    SoundIO::shutdown();
    return 0;
}
