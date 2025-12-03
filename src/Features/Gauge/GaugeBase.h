#pragma once

#include <compare>

class GaugeBase
{
public:
    GaugeBase() = default;
    virtual ~GaugeBase() = default;

    void    operator=(const float& val);
    float&  operator+=(const float& val);
    inline auto operator<=>(const float& val) const noexcept { return currentValue_ <=> val; }

    float GetCurrentValue() const { return currentValue_; }

protected:
    float currentValue_ = 1.0f;
    float targetValue_ = 1.0f;
};
