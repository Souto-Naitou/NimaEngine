#pragma once

#include <cmath>
#include <numbers>

namespace Math
{
    inline float rcp(float _x)
    {
        return 1.0f / _x;
    }

    inline float gauss(float _x, float _y, float _sigma)
    {
        float exponent = -(_x * _x + _y * _y) * rcp(2.0f * _sigma * _sigma);
        float denominator = 2.0f * std::numbers::pi_v<float> * _sigma * _sigma;
        return std:: exp(exponent) * rcp(denominator);
    }
    inline float smoothNoise(float t)
    {
        auto i = static_cast<int>(t);
        float f = t - static_cast<float>(i);

        float a = std::sin(static_cast<float>(i) * 12.9898f) * 43758.5453f;
        float b = std::sin((static_cast<float>(i) + 1.0f) * 12.9898f) * 43758.5453f;
        a = a - std::floor(a);
        b = b - std::floor(b);

        // コサイン補間で滑らかにする 
        float w = (1 - std::cos(f * 3.141592f)) * 0.5f; 
        
        return a * (1 - w) + b * w;
    }
}