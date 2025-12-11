#include "WinSystem.h"
#include <cassert>

#include <NiGui/NiGui.h>
#include <Core/ConfigManager/ConfigManager.h>
#include <Utility/ConvertString/ConvertString.h>

#ifdef _DEBUG
#include <imgui.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

uint32_t WinSystem::clientWidth     = 1600;
uint32_t WinSystem::clientHeight    = 900;
uint32_t WinSystem::preClientWidth  = 1600;
uint32_t WinSystem::preClientHeight = 900;

bool WinSystem::isMoving_           = false;
bool WinSystem::isResized_          = false;
bool WinSystem::isFullScreen_       = false;

void WinSystem::Initialize()
{
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) assert(false && "COMの初期化失敗");

    wc_.lpfnWndProc   = WinSystem::WindowProcedure;
    wc_.lpszClassName = L"DXWindowClass";
    wc_.hInstance     = GetModuleHandle(nullptr);
    wc_.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc_);

    auto& cfgData     = ConfigManager::GetInstance()->GetConfigData();
    clientWidth       = cfgData.screen_width;
    clientHeight      = cfgData.screen_height;
    title_            = ConvertString(cfgData.window_title);
}

void WinSystem::Finalize() const
{
    CloseWindow(hwnd_);
    CoUninitialize();
}

void WinSystem::ShowWnd()
{
    RECT wrc = { 0,0,static_cast<LONG>(clientWidth), static_cast<LONG>(clientHeight) };
    long flag = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;
    AdjustWindowRect(&wrc, flag, false);
    hwnd_ = CreateWindow(
        wc_.lpszClassName,          // 利用するクラス名
        title_.c_str(),             // タイトルバーの文字
        flag,                       // よく見るウィンドウスタイル
        CW_USEDEFAULT,              // 表示X座標(ウィンドウ出現位置？)
        CW_USEDEFAULT,              // 表示Y座標
        wrc.right - wrc.left,       // ウィンドウ横幅
        wrc.bottom - wrc.top,       // ウィンドウ縦幅
        nullptr,                    // 親ウィンドウハンドル(子ウィンドウ作るときに使うかも？)
        nullptr,                    // メニューハンドル
        wc_.hInstance,              // インスタンスハンドル
        nullptr                     // オプション
    );
    ShowWindow(hwnd_, SW_SHOW);
}

UINT WinSystem::GetMsg()
{
    // Windowにメッセージが来てたら最優先で処理させる
    if(PeekMessage(&msg_, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg_);    // メッセージを解釈？
        DispatchMessage(&msg_);    // デキュー的な？
    }
    return msg_.message;
}

void WinSystem::ToggleFullScreen()
{
    isFullScreen_ = !isFullScreen_;
    if(isFullScreen_)
    {
        // フルスクリーンモードにする
        SetWindowLong(hwnd_, GWL_STYLE, WS_POPUP);
        SetWindowPos(hwnd_, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    else
    {
        // ウィンドウモードに戻す
        #ifndef _DEBUG
        auto dwFlag = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        #else
        auto dwFlag = WS_OVERLAPPEDWINDOW;
        #endif // _DEBUG
        SetWindowLong(hwnd_, GWL_STYLE, dwFlag);
        SetWindowPos(hwnd_, HWND_TOP, 100, 100, clientWidth, clientHeight, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

bool WinSystem::IsResized()
{
    if(isResized_)
    {
        isResized_ = false;

        // ウィンドウのサイズを取得
        preClientWidth = clientWidth;
        preClientHeight = clientHeight;

        GetClientRect(hwnd_, &wndSize_);

        if (wndSize_.right - wndSize_.left <= 0 || wndSize_.bottom - wndSize_.top <= 0)
        {
            // ウィンドウのサイズが0以下になってしまった場合は、前回のサイズを使用
            clientWidth = preClientWidth;
            clientHeight = preClientHeight;
        }
        else
        {
            // ウィンドウのサイズを更新
            clientWidth = static_cast<uint32_t>(wndSize_.right - wndSize_.left);
            clientHeight = static_cast<uint32_t>(wndSize_.bottom - wndSize_.top);
        }

        return true;
    }
    return false;
}

LRESULT CALLBACK WinSystem::WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    NiGui::NiGui_WndProcHandler(hwnd, msg, wparam, lparam);

    #ifdef _DEBUG
    if(ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
    {
        return true;
    }
    #endif // _DEBUG

    // メッセージに応じてゲーム固有の処理を行う
    switch(msg)
    {
    case WM_DESTROY:	// ウィンドウが破壊された
        // OSに対して、アプリ終了を伝える
        PostQuitMessage(0);
        return 0;

    case WM_ENTERSIZEMOVE:
        // ウィンドウサイズ変更開始
        isMoving_ = true;
        break;


    case WM_EXITSIZEMOVE:
        // ウィンドウサイズ変更終了
        isMoving_ = false;
        break;

    case WM_SIZE:
        isResized_ = true;
    }

    // 標準のメッセージ処理を行う
    return DefWindowProc(hwnd, msg, wparam, lparam);
}
