#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>

/// <summary>
/// Windowsアプリ統合
/// </summary>
class Window
{
public:
    static uint32_t clientWidth;
    static uint32_t clientHeight;
    static uint32_t preClientWidth;
    static uint32_t preClientHeight;

    Window(const Window&) = delete;
    Window(const Window&&) = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(const Window&&) = delete;

    static Window* GetInstance() { static Window instance;return &instance; };

    HWND GetHwnd() const { return hwnd_; }
    HINSTANCE GetInstanceHandle() const { return wc_.hInstance; }

    void Initialize();
    void Finalize() const;
    void ShowWnd();
    UINT GetMsg();
    void ToggleFullScreen();

    bool IsResized();

private:
    Window() = default;
    ~Window() = default;

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