#pragma once

class GaugeBase
{
public:
    GaugeBase() = default;
    virtual ~GaugeBase() = default;

    inline void    operator=(const float& val)                      { currentValue_ = val; }
    inline float&  operator+=(const float& val)                     { currentValue_ += val; return currentValue_; }
    inline auto    operator<=>(const float& val) const noexcept     { return currentValue_ <=> val; }

    float GetCurrentValue() const { return currentValue_; }

protected:
    float currentValue_ = 1.0f;
};

