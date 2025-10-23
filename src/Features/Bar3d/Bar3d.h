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
  
    /// <summary>
    /// 3D バーを初期化します。
    /// </summary>
    /// <param name="colorContext">バー本体の色。</param>
    /// <param name="colorBG">背景バーの色。</param>
    void Initialize(const RGBA& colorContext = 0xff0000ff, const RGBA& colorBG = 0x000000cc);
    
    /// <summary>
    /// 値や内部状態を更新します。
    /// </summary>
    void Update();
    
    /// <summary>
    /// 2D としてバーを描画します。
    /// </summary>
    void Draw2d();
    /// <summary>
    /// 一定時間バーを表示します。
    /// </summary>
    /// <param name="_sec">表示時間（秒）。</param>
    void Display(float _sec);
    void Display(bool flag) { isDisplay_ = flag; }


public:
    void SetMaxValue(float _value) { valueMax_ = _value; }
    void SetCurrentValue(float _value) { valueCurrent_ = _value; }
    void SetPosition(const Vector2& _position);
    void SetSize(const Vector2& _size);
    void SetEnableTimer(bool flag) { enableTimer_ = flag; }


public:
    /// <summary>
    /// ワールド座標からスクリーン上のヘッドアップ位置を計算します。
    /// </summary>
    /// <param name="_targetPos">対象のワールド座標。</param>
    /// <param name="_hpbarSize">バーサイズ。</param>
    /// <param name="_camera">参照するカメラ。</param>
    /// <param name="_offsetY">Y 方向のオフセット。</param>
    /// <returns>スクリーン座標。</returns>
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