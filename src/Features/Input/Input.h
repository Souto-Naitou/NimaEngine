#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <wrl.h>
#include <cstdint>
#include <Vector2.h>

/// <summary>
/// 入力管理クラス
/// </summary>
class Input
{
public:
    enum class MouseNum
    {
        Left = 0,
        Right = 1,
        Center = 2,
    };

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;
    Input(Input&&) = delete;
    Input& operator=(Input&&) = delete;

    static Input* GetInstance()
    {
        static Input instance;
        return &instance;
    }

    /// <summary>
    /// 入力システムを初期化します。
    /// </summary>
    /// <param name="_hInstance">アプリケーションインスタンス。</param>
    /// <param name="_hwnd">ウィンドウハンドル。</param>
    void        Initialize(HINSTANCE _hInstance, HWND _hwnd);
    /// <summary>
    /// 入力状態を更新します。
    /// キーボード/マウス/パッドの状態を取得して内部に反映します。
    /// </summary>
    void        Update();

    /// <summary>
    /// 入力処理の有効/無効を切り替えます。
    /// </summary>
    /// <param name="_flag">trueで有効。</param>
    void        Enable(bool _flag);

    void        SetDeadZoneRange(float deadZoneRange) { deadZoneRange_ = deadZoneRange; }

    bool        PushKey(BYTE _keyNumber) const;
    bool        PushKeyC(char _key) const;
    bool        TriggerKey(BYTE _keyNumber) const;
    bool        TriggerKeyC(char _key) const;
    Vector2     GetLeftStickPosition() const;
    Vector2     GetRightStickPosition() const;

    bool        PushMouse(MouseNum _mouseNum) const;
    bool        TriggerMouse(MouseNum _mouseNum) const;
    int32_t     GetWheelDelta() const;
    IDirectInput8* GetDirectInput() const { return directInput_.Get(); }
    IDirectInputDevice8** GetPad() { return pad_.GetAddressOf(); }


private:
    Input() = default;
    ~Input() = default;
    
    // Internal functions
    static BOOL CALLBACK EnumJoystickCallback(const DIDEVICEINSTANCE* pdidInstance, void* context);
    /// <summary>
    /// 文字からDirectInputのキー番号へ変換します。
    /// </summary>
    BYTE GetKeyNumber(char _key) const;
    /// <summary>
    /// デバイスの生入力から論理入力へマッピングします。
    /// </summary>
    void MapInputData();
    /// <summary>
    /// ゲームパッドを初期化します。
    /// </summary>
    void InitializePad(HWND hwnd);
    /// <summary>
    /// ゲームパッド状態を更新します。
    /// </summary>
    void UpdatePad();
    /// <summary>
    /// 指定デバイスの状態を更新します。
    /// </summary>
    void UpdateDeviceState(IDirectInputDevice8* pDevice, LPVOID out_state, size_t sizeState);

    // State
    bool isEnable_ = true;

    // Config
    const int32_t kStickRange = 1000; // ジョイスティックの範囲

    // DirectInput data
    Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_ = nullptr;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> pad_ = nullptr;
    DIMOUSESTATE2 mouseState_ = {};
    DIJOYSTATE2 padState_ = {};

    // Logical input data
    BYTE    key_[256]           = {};
    BYTE    keyPre_[256]        = {};
    bool    leftClick_          = false;
    bool    leftClickPre_       = false;
    bool    rightClick_         = false;
    bool    rightClickPre_      = false;
    int32_t wheelDelta_         = 0;
    Vector2 leftStickPosition_  = { 0.0f, 0.0f };
    Vector2 rightStickPosition_ = { 0.0f, 0.0f };
    bool    buttons_[32]        = {}; // ボタンの状態
    bool    buttonsPre_[32]     = {}; // 前回のボタン状態
    float   deadZoneRange_      = 0.1f;  // ジョイスティックのデッドゾーン範囲
};