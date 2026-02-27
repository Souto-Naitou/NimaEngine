#include "Input.h"

#include <cassert>
#include <stdexcept>
#include <Core/Window/Window.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
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

    this->InitializePad(hwnd);
}

void Input::Update()
{
    // 配列をコピー
    memcpy(keyPre_, key_, 256);

    this->UpdateDeviceState(keyboard_.Get(), key_, sizeof(key_));

    // マウスの入力情報を取得
    this->UpdateDeviceState(mouse_.Get(), &mouseState_, sizeof(mouseState_));

    // マウスの状態を更新
    this->UpdateDeviceState(pad_.Get(), &padState_, sizeof(padState_));

    // カーソル位置の更新
    this->UpdateCursorPosition();

    // ゲームパッドの更新
    this->UpdatePad();

    // デシリアライズ
    this->MapInputData();
}

void Input::Enable(bool flag)
{
    isEnable_ = flag;
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

bool Input::ReleaseKey(BYTE keyNumber) const
{
    // キーが離された瞬間ならtrueを返す
    if(!key_[keyNumber] && keyPre_[keyNumber])
    {
        return true;
    }
    return false;
}

Vector2 Input::GetLeftStickPosition() const
{
    return leftStickPosition_;
}

Vector2 Input::GetRightStickPosition() const
{
    return rightStickPosition_;
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

void Input::InitializePad(HWND hwnd)
{
    HRESULT hr = directInput_->EnumDevices(
        DI8DEVTYPE_GAMEPAD, EnumJoystickCallback, (void*)this, DIEDFL_ATTACHEDONLY
    );
    assert(SUCCEEDED(hr));

    if (!pad_) return;

    hr = pad_->SetDataFormat(&c_dfDIJoystick2);
    assert(SUCCEEDED(hr));

    // 軸を絶対値モードに設定
    {
        DIPROPDWORD prop;
        ZeroMemory(&prop, sizeof(prop));
        prop.diph.dwSize = sizeof(prop);
        prop.diph.dwHeaderSize = sizeof(prop.diph);
        prop.diph.dwHow = DIPH_DEVICE;
        prop.diph.dwObj = 0;
        prop.dwData = DIPROPAXISMODE_ABS;
        hr = pad_->SetProperty(DIPROP_AXISMODE, &prop.diph);
    }
    assert(SUCCEEDED(hr));

    // ジョイスティックの範囲を設定
    {
        DIPROPRANGE prop;
        ZeroMemory(&prop, sizeof(prop));
        prop.diph.dwSize = sizeof(prop);
        prop.diph.dwHeaderSize = sizeof(prop.diph);
        prop.diph.dwHow = DIPH_BYOFFSET;
        prop.diph.dwObj = DIJOFS_X;
        prop.lMin = -kStickRange;
        prop.lMax = kStickRange;
        hr = pad_->SetProperty(DIPROP_RANGE, &prop.diph);
        // Y軸の範囲も同様に設定
        prop.diph.dwObj = DIJOFS_Y;
        hr = pad_->SetProperty(DIPROP_RANGE, &prop.diph);
    }
    assert(SUCCEEDED(hr));

    // ジョイスティックの排他制御レベルを設定
    hr = pad_->SetCooperativeLevel(
        hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE
    );
    assert(SUCCEEDED(hr));

    // ジョイスティックの情報の取得を開始
    hr = pad_->Acquire();
    assert(SUCCEEDED(hr));
}

void Input::UpdatePad()
{
    if (!pad_) return;
    leftStickPosition_.x = padState_.lX / static_cast<float>(kStickRange); // X軸の値を-1.0fから1.0fに変換
    leftStickPosition_.y = padState_.lY / static_cast<float>(kStickRange); // Y軸の値を-1.0fから1.0fに変換
    rightStickPosition_.x = padState_.lRx / static_cast<float>(kStickRange); // 右スティックX軸
    rightStickPosition_.y = padState_.lRy / static_cast<float>(kStickRange); // 右スティックY軸

    // ボタンの状態を更新
    for (int i = 0; i < 32; ++i)
    {
        if (padState_.rgbButtons[i] & 0x80)
        {
            buttons_[i] = true; // ボタンが押されている
        }
        else
        {
            buttons_[i] = false; // ボタンが離されている
        }
    }
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

int Input::EnumJoystickCallback(const DIDEVICEINSTANCE* pdidInstance, void* context)
{
    auto pInput = static_cast<Input*>(context);
    auto directInput = pInput->GetDirectInput();
    
    if (FAILED(directInput->CreateDevice(pdidInstance->guidInstance, pInput->GetPad(), nullptr)))
    {
        return DIENUM_CONTINUE; // エラーが発生した場合は次のデバイスへ
    }
    return DIENUM_STOP; // ジョイスティックが見つかったので列挙を停止
}
