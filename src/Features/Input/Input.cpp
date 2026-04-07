#include "Input.h"

#include <Core/Window/Window.h>
#include <imgui.h>
#include <Features/event/EventListener.h>
#include <event/InputCallbackEvent.h>

#include <stdexcept>
#include <cassert>
#include <bitset>
#include <sstream>

#include <Xinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput9_1_0.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
    pDebugEntry_ = std::make_unique<DebugEntry<Input>>("Input", this, false);

    HRESULT hr = DirectInput8Create(
        hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, static_cast<void**>(&directInput_), nullptr
    );
    assert(SUCCEEDED(hr));

    // キーボードデバイスの生成
    hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, nullptr);
    assert(SUCCEEDED(hr));

    // 入力データ形式のセット
    hr = keyboard_->SetDataFormat(&c_dfDIKeyboard); // 標準形式
    assert(SUCCEEDED(hr));

    // 排他制御レベルのセット
    hr = keyboard_->SetCooperativeLevel(
        hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE
    );
    assert(SUCCEEDED(hr));

    // キーボード情報の取得開始
    keyboard_->Acquire();

    // マウスデバイスの生成
    hr = directInput_->CreateDevice(GUID_SysMouse, &mouse_, nullptr);
    assert(SUCCEEDED(hr));

    // 入力データ形式のセット
    hr = mouse_->SetDataFormat(&c_dfDIMouse2);
    assert(SUCCEEDED(hr));

    // 排他制御レベルのセット
    hr = mouse_->SetCooperativeLevel(
        hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE
    );

    // マウス情報の取得開始
    mouse_->Acquire();

    assert(SUCCEEDED(hr));
}

void Input::UpdateInputDeviceState()
{
    // 配列をコピー
    memcpy(keyPre_, key_, 256);

    this->UpdateDeviceState(keyboard_.Get(), key_, sizeof(key_));

    // マウスの入力情報を取得
    this->UpdateDeviceState(mouse_.Get(), &mouseState_, sizeof(mouseState_));

    // カーソル位置の更新
    this->UpdateCursorPosition();

    // ゲームパッドの更新
    this->UpdatePad();

    // デシリアライズ
    this->MapInputData();

    // ゲームパッドが接続されていて更新されている場合はゲームパッドモードに切り替える
    if (isPadConnected_ && isPadUpdated_)
    {
        isPadMode_ = true;
    }
    
    // キーボードやマウスの入力があった場合はゲームパッドモードを解除する
    if (this->IsAnyKeyChanged())
    {
        isPadMode_ = false;
    }
}

void Input::Update()
{
    if (padIndex_ == -1) return;
    /// バイブレーションの確定処理
    padVibration_.wLeftMotorSpeed = static_cast<WORD>(padVibrationNormalized_.x * 65535.0f);
    padVibration_.wRightMotorSpeed = static_cast<WORD>(padVibrationNormalized_.y * 65535.0f);
    XInputSetState(padIndex_, &padVibration_);
    padVibrationNormalized_ = Vector2();
}

void Input::Enable(bool flag)
{
    isEnable_ = flag;
}

void Input::Vibrate(float leftMotorSpeed, float rightMotorSpeed)
{
    padVibrationNormalized_ = Vector2(leftMotorSpeed, rightMotorSpeed);
}

void Input::SetGamepadVibrationLeft(float leftMotorSpeed)
{
    padVibrationNormalized_.x = leftMotorSpeed;
}

void Input::SetGamepadVibrationRight(float rightMotorSpeed)
{
    padVibrationNormalized_.y = rightMotorSpeed;
}

bool Input::PushKey(BYTE keyNumber) const
{
    // 指定キーを押して入ればtrueを返す
    if (key_[keyNumber])
    {
        return true;
    }
    return false;
}

bool Input::PushKeyC(char key) const
{
    // 指定キーを押して入ればtrueを返す
    if(key_[GetKeyNumber(key)])
    {
        return true;
    }
    return false;
}

bool Input::PushButton(WORD buttonNum) const
{
    if (!isPadConnected_) return false;
    return (padState_.Gamepad.wButtons & buttonNum) != 0;
}

bool Input::TriggerKey(BYTE keyNumber) const
{
    // キーが押された瞬間ならtrueを返す
    if(key_[keyNumber] && !keyPre_[keyNumber])
    {
        return true;
    }
    return false;
}

bool Input::TriggerKeyC(char key) const
{
    // キーが押された瞬間ならtrueを返す
    BYTE keyNumber = GetKeyNumber(key);
    if(key_[keyNumber] && !keyPre_[keyNumber])
    {
        return true;
    }
    return false;
}

bool Input::TriggerButton(WORD buttonNum) const
{
    if (!isPadConnected_) return false;
    bool isPressed = (padState_.Gamepad.wButtons & buttonNum) != 0;
    bool wasPressed = (padStatePrev_.Gamepad.wButtons & buttonNum) != 0;

    if (isPressed && !wasPressed)
    {
        return true;
    }
    return false;
}

bool Input::ReleaseKey(BYTE keyNumber) const
{
    // キーが離された瞬間ならtrueを返す
    if(!key_[keyNumber] && keyPre_[keyNumber])
    {
        return true;
    }
    return false;
}

bool Input::ReleaseButton(WORD buttonNum) const
{
    if (!isPadConnected_) return false;

    bool isPressed = (padState_.Gamepad.wButtons & buttonNum) != 0;
    bool wasPressed = (padStatePrev_.Gamepad.wButtons & buttonNum) != 0;

    if (!isPressed && wasPressed)
    {
        return true;
    }
    return false;
}

bool Input::ReleaseMouse(MouseNum mouseNum) const
{
    if (mouseNum == MouseNum::Left)
    {
        if (!leftClick_ && leftClickPre_)
        {
            return true;
        }
    }
    else if (mouseNum == MouseNum::Right)
    {
        if (!rightClick_ && rightClickPre_)
        {
            return true;
        }
    }

    return false;
}

bool Input::IsPadConnected() const
{
    return isPadConnected_;
}

bool Input::IsPadUpdated() const
{
    return isPadUpdated_;
}

void Input::ImGui()
{
    #ifdef _DEBUG

    ImGui::SeparatorText("Gamepad Connection");
    if (isPadConnected_)
    {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");

        if (ImGui::TreeNode("Gamepad Information"))
        {
            if (ImGui::TreeNode("Raw Gamepad State"))
            {
                // パケット
                ImGui::Text("packet : %d", padState_.dwPacketNumber);
                ImGui::Text("Last updated : %.2fsec ago", timePadNonUpdate_.GetNow<float>());

                auto& gamepad = padState_.Gamepad;
                // ボタンの生データ (2進数で表示)
                std::stringstream buttonsBinaryString;
                buttonsBinaryString << std::bitset<16>(gamepad.wButtons);
                ImGui::Text("buttons : 0b%s", buttonsBinaryString.str().c_str());
                // スティックの生データ
                ImGui::Text("Left Stick : (%.5d,%.5d)", gamepad.sThumbLX, gamepad.sThumbLY);
                ImGui::Text("Right Stick : (%.5d,%.5d)", gamepad.sThumbRX, gamepad.sThumbRY);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Formatted Gamepad State"))
            {
                ImGui::Text("Left Stick : (%f,%f)", gamepadAnalogInput_.thumbL.x, gamepadAnalogInput_.thumbL.y);
                ImGui::Text("Right Stick : (%f,%f)", gamepadAnalogInput_.thumbR.x, gamepadAnalogInput_.thumbR.y);

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Disconnected");
    }

    if (ImGui::TreeNode("Gamepad Setting"))
    {
        ImGui::SeparatorText("Deadzone");
        ImGui::SliderFloat("Trigger Left", &deadzone_.triggerL, 0.0f, 1.0f);
        ImGui::SliderFloat("Trigger Right", &deadzone_.triggerR, 0.0f, 1.0f);
        ImGui::SliderFloat("Thumb Left", &deadzone_.thumbL, 0.0f, 1.0f);
        ImGui::SliderFloat("Thumb Right", &deadzone_.thumbR, 0.0f, 1.0f);

        ImGui::SeparatorText("Vibration");
        ImGui::SliderFloat("Left Motor", &padVibrationNormalized_.x, 0.0f, 1.0f);
        ImGui::SliderFloat("Right Motor", &padVibrationNormalized_.y, 0.0f, 1.0f);

        ImGui::TreePop();
    }
    #endif // _DEBUG
}

POINT Input::GetCursorPosition() const
{
    return mousePosition_;
}

bool Input::PushMouse(MouseNum mouseNum) const
{
    if (mouseNum == MouseNum::Left)
    {
        if (leftClick_)
        {
            return true;
        }
    }
    else if (mouseNum == MouseNum::Right)
    {
        if (rightClick_)
        {
            return true;
        }
    }

    return false;
}

bool Input::TriggerMouse(MouseNum mouseNum) const
{
    if (mouseNum == MouseNum::Left)
    {
        if (leftClick_ && !leftClickPre_)
        {
            return true;
        }
    }
    else if (mouseNum == MouseNum::Right)
    {
        if (rightClick_ && !rightClickPre_)
        {
            return true;
        }
    }

    return false;
}

int32_t Input::GetWheelDelta() const
{
    return wheelDelta_;
}

bool Input::IsAnyKeyChanged() const
{
    return std::memcmp(key_, keyPre_, 256) != 0;
}

BYTE Input::GetKeyNumber(char key) const
{
    // キーの文字からキー番号を取得
    return static_cast<BYTE>(MapVirtualKey(key, MAPVK_VK_TO_VSC));
}

void Input::MapInputData()
{
    leftClickPre_ = leftClick_;
    rightClickPre_ = rightClick_;

    if (!isEnable_ && !leftClick_ && !rightClick_)
    {
        leftClick_ = false;
        rightClick_ = false;
        wheelDelta_ = 0;
        return;
    }

    // 左クリックの入力情報を取得
    if (mouseState_.rgbButtons[0] & 0x80)
    {
        leftClick_ = true;
    }
    else
    {
        leftClick_ = false;
    }

    // 右クリックの入力情報を取得
    if (mouseState_.rgbButtons[1] & 0x80)
    {
        rightClick_ = true;
    }
    else
    {
        rightClick_ = false;
    }

    wheelDelta_ = mouseState_.lZ;
}

void Input::UpdatePad()
{
    padStatePrev_ = padState_;

    bool wasPadConnected = isPadConnected_;
    isPadConnected_ = false;

    /// コントローラーの接続状態を確認
    DWORD dwResult;
    for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
    {
        ZeroMemory(&padState_, sizeof(XINPUT_STATE));
        dwResult = XInputGetState(i, &padState_);
        if (dwResult == ERROR_SUCCESS)
        {
            /// コントローラーが接続されている
            isPadConnected_ = true;

            /// 初めて接続されたコントローラーなら振動をリセット
            if (padIndex_ == -1)
            {
                XInputSetState(i, &padVibration_);
            }
            padIndex_ = static_cast<int32_t>(i); // iは0～3
            break;
        }
    }

    /// コントローラーの接続状態が変化した場合、イベントを発行
    bool isConnectedNow = isPadConnected_ && !wasPadConnected;
    bool isDisconnectedNow = !isPadConnected_ && wasPadConnected;

    if (isConnectedNow)
    {
        EventListener::GetInstance()->Publish(Events::GamePadConnected());
    }
    else if (isDisconnectedNow)
    {
        EventListener::GetInstance()->Publish(Events::GamePadDisconnected());
    }

    /// コントローラが接続されていない場合はここで終わり
    if (!isPadConnected_)
    {
        isPadUpdated_ = false;
        return;
    }

    /// コントローラーの状態が更新されたかどうかを判定
    isPadUpdated_ = padState_.dwPacketNumber != padStatePrev_.dwPacketNumber;
    if (isPadUpdated_)
    {
        timePadNonUpdate_.Reset();
        timePadNonUpdate_.Start();
    }

    auto& gamepad = padState_.Gamepad;
    auto& analog = gamepadAnalogInput_;
    analog.thumbL.x    = std::max(-1.0f, static_cast<float>(gamepad.sThumbLX) / 32767.0f); // X軸の値を-1.0fから1.0fに変換
    analog.thumbL.y    = std::max(-1.0f, static_cast<float>(gamepad.sThumbLY) / 32767.0f); // Y軸の値を-1.0fから1.0fに変換
    analog.thumbR.x    = std::max(-1.0f, static_cast<float>(gamepad.sThumbRX) / 32767.0f); // 右スティックX軸
    analog.thumbR.y    = std::max(-1.0f, static_cast<float>(gamepad.sThumbRY) / 32767.0f); // 右スティックY軸
    analog.triggerL    = static_cast<float>(gamepad.bLeftTrigger) / 255.0f; // 左トリガーの値を0.0fから1.0fに変換
    analog.triggerR    = static_cast<float>(gamepad.bRightTrigger) / 255.0f; // 右トリガーの値を0.0fから1.0fに変換

    /// デッドゾーンの処理
    if (gamepadAnalogInput_.thumbL.LengthWithoutRoot() <= deadzone_.thumbL) gamepadAnalogInput_.thumbL = Vector2();
    if (gamepadAnalogInput_.thumbR.LengthWithoutRoot() <= deadzone_.thumbR) gamepadAnalogInput_.thumbR = Vector2();
    if (gamepadAnalogInput_.triggerL <= deadzone_.triggerL) gamepadAnalogInput_.triggerL = 0.0f;
    if (gamepadAnalogInput_.triggerR <= deadzone_.triggerR) gamepadAnalogInput_.triggerR = 0.0f;
}

void Input::UpdateDeviceState(IDirectInputDevice8* pDevice, LPVOID out_state, size_t sizeState)
{
    if (!pDevice) return;

    // デバイスの状態
    HRESULT hr = pDevice->Poll();

    if (FAILED(hr))
    {
        // デバイスが応答しない場合は再取得
        hr = pDevice->Acquire();
        hr = pDevice->Poll();
    }

    if (FAILED(hr)) return;
    
    hr = pDevice->GetDeviceState(static_cast<int>(sizeState), out_state);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to get device state.");
    }
}

void Input::UpdateCursorPosition()
{
    POINT point;
    if (GetCursorPos(&point))
    {
        ScreenToClient(Window::GetInstance()->GetHwnd(), &point);
        point.x -= viewportOffset_.x;
        point.y -= viewportOffset_.y;
        mousePosition_ = point;
    }

    // 異なる画面サイズ用に補正をかける (縦横比が同じである前提)
    if (viewportSize_.cy == 0) return; // ゼロ除算防止
    float ratio = static_cast<float>(Window::clientHeight) / static_cast<float>(viewportSize_.cy);
    mousePosition_.x = static_cast<LONG>(static_cast<float>(mousePosition_.x) * ratio);
    mousePosition_.y = static_cast<LONG>(static_cast<float>(mousePosition_.y) * ratio);
}
