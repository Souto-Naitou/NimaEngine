#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <Xinput.h>
#include <dinput.h>
#include <wrl.h>
#include <cstdint>
#include <Vector2.h>
#include <list>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <memory>
#include <limits>

#undef max

/// ## コントローラの接続時/切断時のコールバック
/// コントローラの接続状態が変化したときにイベントを発行します。
/// コントローラが接続されたときはEvents::GamePadConnectedイベントを、切断されたときはEvents::GamePadDisconnectedイベントを発行します。
/// EventListenerクラスを使用してイベントを購読してください。
/// ## コントローラのスティックやトリガーの取得について
/// コントローラのスティックやトリガーの値は、GamepadAnalogInput構造体にまとめられています。
/// 一般にゲームで使用する場合は、GetGamepadAnalogInput関数を使用して、スティックやトリガーの値を取得してください。
/// この関数で取得できる値は、デッドゾーンを考慮して補正された値です。
/// 例外的に、スティックやトリガーの生の値が必要な場合は、GetRawGamepadState関数を使用して、XINPUT_GAMEPAD構造体から取得してください。

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

    struct Deadzone
    {
        float thumbL = 0.1f;
        float thumbR = 0.1f;
        float triggerL = 0.1f;
        float triggerR = 0.1f;
    };

    struct GamepadAnalogInput
    {
        Vector2 thumbL = {};
        Vector2 thumbR = {};
        float triggerL = 0.0f;
        float triggerR = 0.0f;
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
    /// <param name="hInstance">アプリケーションインスタンス。</param>
    /// <param name="hwnd">ウィンドウハンドル。</param>
    void        Initialize(HINSTANCE hInstance, HWND hwnd);

    /// <summary>
    /// 入力状態を更新します。
    /// キーボード/マウス/パッドの状態を取得して内部に反映します。
    /// </summary>
    void        Update();

    /// <summary>
    /// 入力処理の有効/無効を切り替えます。
    /// </summary>
    /// <param name="flag">trueで有効。</param>
    void        Enable(bool flag);

    void        SetWindowOffset(POINT offset) { viewportOffset_ = offset; }
    void        SetWindowSize(SIZE size) { viewportSize_ = size; }

    Deadzone&   GetDeadzone() { return deadzone_; }
    const Deadzone& GetDeadzone() const { return deadzone_; }
    const GamepadAnalogInput& GetGamepadAnalogInput() const { return gamepadAnalogInput_; }
    const XINPUT_GAMEPAD& GetRawGamepadState() const { return padState_.Gamepad; }

    /// 状態の取得
    bool        PushKey(BYTE keyNumber) const;
    bool        PushKeyC(char key) const;
    bool        PushButton(BYTE buttonNum) const;
    bool        PushMouse(MouseNum mouseNum) const;

    bool        TriggerKey(BYTE keyNumber) const;
    bool        TriggerKeyC(char key) const;
    bool        TriggerButton(BYTE buttonNum) const;
    bool        TriggerMouse(MouseNum mouseNum) const;

    bool        ReleaseKey(BYTE keyNumber) const;
    bool        ReleaseButton(BYTE buttonNum) const;

    POINT       GetCursorPosition() const;
    int32_t     GetWheelDelta() const;

    bool        IsAnyKeyChanged() const;
    bool        IsPadConnected() const;
    bool        IsPadUpdated() const;

    IDirectInput8* GetDirectInput() const { return directInput_.Get(); }
    IDirectInputDevice8** GetPad() { return pad_.GetAddressOf(); }

    void        ImGui();

private:
    Input() = default;
    ~Input() = default;
    
    /// <summary>
    /// 文字からDirectInputのキー番号へ変換します。
    /// </summary>
    BYTE GetKeyNumber(char key) const;

    /// <summary>
    /// デバイスの生入力から論理入力へマッピングします。
    /// </summary>
    void MapInputData();

    /// <summary>
    /// ゲームパッド状態を更新します。
    /// </summary>
    void UpdatePad();

    /// <summary>
    /// 指定デバイスの状態を更新します。
    /// </summary>
    void UpdateDeviceState(IDirectInputDevice8* pDevice, LPVOID out_state, size_t sizeState);

    /// <summary>
    /// カーソル位置を更新します。
    /// </summary>
    void UpdateCursorPosition();

    // デバッグエントリ
    std::unique_ptr<DebugEntry<Input>> pDebugEntry_ = nullptr;

    // State
    bool isEnable_ = true;

    // DirectInput data
    Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_ = nullptr;
    Microsoft::WRL::ComPtr<IDirectInputDevice8> pad_ = nullptr;
    DIMOUSESTATE2   mouseState_ = {};
    XINPUT_STATE    padState_ = {};
    XINPUT_STATE    padStatePrev_ = {};

    bool    isPadConnected_     = false;
    bool    isPadUpdated_       = false;

    // コントローラの入力が最後に更新されてからの時間を測るためのタイマー
    TimeMeasurer            timePadNonUpdate_       = {};

    // Logical input data
    BYTE                    key_[256]               = {};
    BYTE                    keyPre_[256]            = {};
    bool                    leftClick_              = false;
    bool                    leftClickPre_           = false;
    bool                    rightClick_             = false;
    bool                    rightClickPre_          = false;
    int32_t                 wheelDelta_             = 0;
    Deadzone                deadzone_               = {};
    GamepadAnalogInput      gamepadAnalogInput_     = {};
    POINT                   mousePosition_          = { 0, 0 };
    POINT                   viewportOffset_         = { 0, 0 };
    SIZE                    viewportSize_           = { 0, 0 };
};