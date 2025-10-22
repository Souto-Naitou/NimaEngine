#pragma once

#include <Features/Sprite/Sprite.h>
#include <Color.h>
#include <memory>
#include <Features/TimeMeasurer/TimeMeasurer.h>
#include <Features/GameEye/GameEye.h>

/// <summary>
/// 3Dバー表示クラス
/// </summary>
class Bar3d
{
public:
    Bar3d() = default;
    ~Bar3d() = default;
  
    void Initialize(const RGBA& colorContext = 0xff0000ff, const RGBA& colorBG = 0x000000cc);
    void Update();
    void Draw2d();
    void Display(float _sec);
    void Display(bool flag) { isDisplay_ = flag; }


public:
    void SetMaxValue(float _value) { valueMax_ = _value; }
    void SetCurrentValue(float _value) { valueCurrent_ = _value; }
    void SetPosition(const Vector2& _position);
    void SetSize(const Vector2& _size);
    void SetEnableTimer(bool flag) { enableTimer_ = flag; }


public:
    static Vector2 GetHeadUpPositionOnScreen(
        const Vector3& _targetPos, 
        const Vector2& _hpbarSize,
        const GameEye& _camera,
        float _offsetY = 0.0f);

private:
    std::unique_ptr<Sprite> sprite_current_;
    std::unique_ptr<Sprite> sprite_max_;

private:
    float                   valueMax_       = 0.0f;
    float                   valueCurrent_   = 0.0f;

    TimeMeasurer            timerDisplay_   = {};
    bool                    isDisplay_      = false;
    bool                    enableTimer_    = true;
    float                   displayTime_    = 0.0f;


private:
    void InitializeSprite(std::unique_ptr<Sprite>& _sprite, const RGBA& _color);
    void CurrentValueSpriteUpdate();
    void UpdateDisplayFlagByTimer();
};