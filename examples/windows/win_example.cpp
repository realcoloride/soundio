#include <windows.h>
#include <iostream>
#include "../../src/SoundIO.h"


std::wstring utf8_to_wstring(const std::string& utf8)
{
    if (utf8.empty()) return std::wstring();

    int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                  utf8.data(), (int)utf8.size(),
                                  nullptr, 0);
    if (len == 0) throw std::runtime_error("MultiByteToWideChar size failed");

    std::wstring wide(len, L'\0');
    int written = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                      utf8.data(), (int)utf8.size(),
                                      wide.data(), len);
    if (written == 0) throw std::runtime_error("MultiByteToWideChar convert failed");
    return wide;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // 1) Init
    if (SoundIO::initialize() != MA_SUCCESS) {
        MessageBoxW(NULL, L"SoundIO::initialize() failed", L"Error", MB_ICONERROR);
        return 1;
    }

    // 2) Use it
    if (auto* device = SoundIO::getDefaultSpeaker()) {
        MessageBoxW(NULL, utf8_to_wstring(device->name).c_str(), L"Success", MB_ICONINFORMATION);
    } else {
        MessageBoxW(NULL, L"No default playback device", L"Error", MB_ICONWARNING);
    }
    

    auto* mic = SoundIO::getDefaultMicrophone();
    mic->subscribe();

    // 3) Shutdown (be nice)
    SoundIO::shutdown();
    return 0;
}
