#pragma once

#include <compare>

class GaugeBase
{
public:
    GaugeBase() = default;
    virtual ~GaugeBase() = default;

    void    operator=(float val);
    float&  operator+=(const float& val);
    inline auto operator<=>(const float& val) const noexcept { return currentValue_ <=> val; }

    float GetRawValue() const { return rawValue_; }
    float GetCurrentValue() const { return currentValue_; }

protected:
    // 生の値(0.0f~1.0f)
    float rawValue_ = 1.0f;
    // 補間後の現在値(0.0f~1.0f)
    float currentValue_ = 1.0f;
};
