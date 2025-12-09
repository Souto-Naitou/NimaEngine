#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>

/// <summary>
/// Windowsアプリ統合
/// </summary>
class WinSystem
{
public:
    static uint32_t clientWidth;
    static uint32_t clientHeight;
    static uint32_t preClientWidth;
    static uint32_t preClientHeight;

    WinSystem(const WinSystem&) = delete;
    WinSystem(const WinSystem&&) = delete;
    WinSystem& operator=(const WinSystem&) = delete;
    WinSystem& operator=(const WinSystem&&) = delete;

    static WinSystem* GetInstance() { static WinSystem instance;return &instance; };

    HWND GetHwnd() const { return hwnd_; }
    HINSTANCE GetInstanceHandle() const { return wc_.hInstance; }

    void Initialize();
    void Finalize() const;
    void ShowWnd();
    UINT GetMsg();
    void ToggleFullScreen();

    bool IsResized();

private:
    WinSystem() = default;
    ~WinSystem() = default;

    static LRESULT __stdcall WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    WNDCLASS        wc_         = {};
    HWND            hwnd_       = {};
    MSG             msg_        = {};
    RECT            wndSize_    = {};
    std::wstring    title_      = {};
    static bool isMoving_;
    static bool isResized_;
    static bool isFullScreen_;
};