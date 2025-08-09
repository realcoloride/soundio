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
    if (SoundIO::initialize() != MA_SUCCESS) {
        MessageBoxW(NULL, L"SoundIO::initialize() failed", L"Error", MB_ICONERROR);
        return 1;
    }

    auto* speaker = SoundIO::getDefaultSpeaker();
    auto* mic = SoundIO::getDefaultMicrophone();

    if (speaker) {
        MessageBoxW(NULL, utf8_to_wstring(mic->name).c_str(), L"Success", MB_ICONINFORMATION);
        MessageBoxW(NULL, utf8_to_wstring(speaker->name).c_str(), L"Success", MB_ICONINFORMATION);
    }
    else {
        MessageBoxW(NULL, L"No default playback device", L"Error", MB_ICONWARNING);
    }

    if (mic && speaker) {
        // Ensure devices are awake before wiring; logs will confirm formats.
        ma_result ar = speaker->ensureAwake();
        if (ar != MA_SUCCESS) MessageBoxW(NULL, L"Speaker wakeUp failed", L"Error", MB_ICONERROR);
        ar = mic->ensureAwake();
        if (ar != MA_SUCCESS) MessageBoxW(NULL, L"Mic wakeUp failed", L"Error", MB_ICONERROR);

        mic->subscribe(speaker); // start the background work
    }

    RegisterHotKey(NULL, 1, 0, VK_ESCAPE); // press ESC to quit
    MSG msg{};
    bool running = true;
    while (running && GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == 1) {
            running = false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnregisterHotKey(NULL, 1);

    SoundIO::shutdown();
    return 0;
}
