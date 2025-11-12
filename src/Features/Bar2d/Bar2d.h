#pragma once
#include <memory>
#include <drawable/sprite/Sprite.h>
#include <vector>
#include <Features/Layer/Canvas.h>

#include <Vector4.h>
#include <Vector2.h>
#include <array>
#include <Color.h>
#include <string>

#define rgba(r, g, b, a) NiVec4{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f}

struct Bar2dInitParams
{
    std::string nameTexturePath = {};
    Vector2     barSize = { 100.0f , 10.0f };
    bool        enableSmoothingColor = true;
};

/// <summary>
/// 2Dバー表示クラス
/// </summary>
class Bar2d
{
public:
    Bar2d() = default;
    ~Bar2d();

    /// <summary>
    /// バー表示を初期化します。
    /// </summary>
    /// <param name="_nameTexturePath">名称テクスチャの相対パス。</param>
    /// <param name="_barSize">バーのサイズ。</param>
    /// <param name="_enable_smoothing_color">色補間を有効にするか。</param>
    void Initialize(const Bar2dInitParams& params);

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize();
    
    /// <summary>
    /// 値や見た目の更新を行います。
    /// </summary>
    void Update();
    
    /// <summary>
    /// 2D 描画を行います。
    /// </summary>
    void Draw1F();
    
    /// <summary>
    /// デバッグ用 UI を描画します。
    /// </summary>
    void ImGui();

public:
    void SetOpacity(float alpha);
    void SetPosition(const Vector2& _pos) { position_ = _pos; }
    void SetAnchorPoint(const Vector2& _anchor) { anchor_ = _anchor; }
    void SetMaxValue(float _max) { maxValue_ = _max; }
    void SetCurrentValue(float _current) { currentValue_ = _current; }

    float GetCurrentValue() const { return currentValue_; }
    float GetMaxValue() const { return maxValue_; }

    void operator=(const float& _val) { currentValue_ = _val; }
    float& operator+=(const float& _val) { currentValue_ += _val; return currentValue_; }

private:
    using NumberImage = std::pair<uint32_t, std::unique_ptr<Sprite>>;

    static constexpr char PATH_BAR[] = "white1x1.png";
    static constexpr char PATH_DECORATION[] = "white1x1.png";
    static const RGBA COLOR_BAR_NORMAL;
    static const RGBA COLOR_BAR_LOW;
    static const RGBA COLOR_BAR_BG;
    static const Vector2 SPACING_HEAD_TO_DECO;
    static constexpr float BORDER_DANGER = 0.25f;

    std::unique_ptr<DebugEntry<Bar2d>> pDebugEntry_ = nullptr;

    Bar2dInitParams initParams_ = {};

    std::unique_ptr<Sprite> bar_ = nullptr;
    std::unique_ptr<Sprite> name_ = nullptr;
    std::unique_ptr<Sprite> background_ = nullptr;

    Vector2 position_ = { 0.0f, 0.0f };
    Vector2 size_ = { 0.0f, 0.0f };
    Vector2 anchor_ = { 0.0f, 0.0f };

    float maxValue_     = 0.0f;
    float currentValue_ = 0.0f;

    bool isDisplay_name_ = true;

private:
    void UpdateTransform();
    void UpdateColor();

    Vector2 mul(const Vector2& lv, const Vector2& rv);
};