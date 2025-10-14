#pragma once
#include <memory>
#include <Features/Sprite/Sprite.h>
#include <vector>

#include <Vector4.h>
#include <Vector2.h>
#include <array>
#include <Color.h>
#include <string>

#define rgba(r, g, b, a) NiVec4{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f}

class Bar2d
{
public:
    Bar2d() = default;
    ~Bar2d() = default;

    void Initialize(const std::string& _nameTexturePath, const Vector2& _barSize, bool _enable_smoothing_color = true);
    void Update();
    void Draw2D();
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
    Vector2 barSize_ = { 100.0f , 10.0f };

    static constexpr float BORDER_DANGER = 0.25f;

    std::unique_ptr<Sprite> bar_ = nullptr;
    std::unique_ptr<Sprite> name_ = nullptr;
    std::unique_ptr<Sprite> background_ = nullptr;
    std::vector<NumberImage> numbers_;

    std::string nameTexturePath_ = {};

    Vector2 position_ = { 0.0f, 0.0f };
    Vector2 size_ = { 0.0f, 0.0f };
    Vector2 anchor_ = { 0.0f, 0.0f };

    float maxValue_ = 0.0f;
    float currentValue_ = 0.0f;

    bool isEnable_lerp_color_ = false;
    bool isDisplay_name_ = true;

private:
    void UpdateTransform();
    void UpdateColor();

    Vector2 mul(const Vector2& lv, const Vector2& rv);
};