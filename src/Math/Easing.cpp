// Copyright © 2024 Souto-Naitou. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#include "Easing.h"
#include <cmath>
#include <numbers>

float pi_float = static_cast<float>(std::numbers::pi);

using namespace Math;

float Easing::EaseNone(float t)
{
    return t;
}

float Easing::EaseInSine(float t)
{
    return 1.0f - std::cosf((t * pi_float) / 2.0f);

}

float Easing::EaseOutSine(float t)
{
    return std::sinf(t * pi_float / 2.0f);

}

float Easing::EaseInOutSine(float t)
{
    return -(std::cosf(t * pi_float) - 1.0f) / 2.0f;

}

float Easing::EaseInQuad(float t)
{
    return t * t;

}

float Easing::EaseOutQuad(float t)
{
    return 1.0f - (1.0f - t) * (1.0f - t);

}

float Easing::EaseInOutQuad(float t)
{
    if (t < 0.5f)
    {
        return 2.0f * t * t;
    }
    else
    {
        return 1.0f - std::powf(-2.0f * t + 2.0f, 2.0f) / 2;
    }

}

float Easing::EaseInCubic(float t)
{
    return t * t * t;

}

float Easing::EaseOutCubic(float t)
{
    return 1.0f - std::powf(1.0f - t, 3.0f);

}

float Easing::EaseInOutCubic(float t)
{
    if (t < 0.5f)
    {
        return 4.0f * t * t * t;
    }
    else
    {
        return 1.0f - std::powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    }

}

float Easing::EaseInQuart(float t)
{
    return t * t * t * t;

}

float Easing::EaseOutQuart(float t)
{
    return 1.0f - std::powf(1.0f - t, 4.0f);

}

float Easing::EaseInOutQuart(float t)
{
    if (t < 0.5f)
    {
        return 8.0f * t * t * t * t;
    }
    else
    {
        return 1.0f - std::powf(-2.0f * t + 2.0f, 4.0f) / 2.0f;
    }

}

float Easing::EaseInBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    return c3 * t * t * t - c1 * t * t;

}

float Easing::EaseOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;

    return 1.0f + c3 * std::powf(t - 1.0f, 3.0f) + c1 * std::powf(t - 1.0f, 2.0f);

}

float Easing::EaseInOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;

    if (t < 0.5f)
    {
        return (std::powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f;
    }
    else
    {
        return (std::powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }

}

float Easing::EaseInElastic(float t)
{
    const float c4 = (2.0f * pi_float) / 3.0f;
    if (t == 0)
    {
        return 0.0f;
    }
    else if (t == 1.0f)
    {
        return 1.0f;
    }
    else
    {
        return -std::powf(2.0f, 10.0f * t - 10.0f) * std::sinf((t * 10.0f - 10.75f) * c4);
    }


}

float Easing::EaseOutElastic(float t)
{
    const float c4 = (2.0f * pi_float) / 3.0f;
    if (t == 0)
    {
        return 0.0f;
    }
    else if (t == 1.0f)
    {
        return 1.0f;
    }
    else
    {
        return std::powf(2.0f, -10.0f * t) * std::sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
    }


}

float Easing::EaseInOutElastic(float t)
{
    const float c5 = (2.0f * pi_float) / 4.5f;

    if (t == 0.0f)
    {
        return 0.0f;
    }
    else if (t == 1.0f)
    {
        return 1.0f;
    }
    else if (t < 0.5f)
    {
        return -(std::powf(2.0f, 20.0f * t - 10.0f) * std::sinf((20.0f * t - 11.125f) * c5)) / 2.0f;
    }
    else
    {
        return (std::powf(2.0f, -20.0f * t + 10.0f) * std::sinf((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
    }

}

float Easing::EaseInBounce(float t)
{
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    float resultT = 0.0f;
    float time = 1.0f - t;

    if (time < 1.0f / d1)
    {
        resultT = n1 * time * time;
    }
    else if (time < 2.0f / d1)
    {
        resultT = n1 * (time -= 1.5f / d1) * time + 0.75f;
    }
    else if (time < 2.5f / d1)
    {
        resultT = n1 * (time -= 2.25f / d1) * time + 0.9375f;
    }
    else
    {
        resultT = n1 * (time -= 2.625f / d1) * time + 0.984375f;
    }

    return 1.0f - resultT;

}

float Easing::EaseOutBounce(float t)
{
    const float n1 = 7.5625f;
    const float d1 = 2.75f;

    if (t < 1.0f / d1)
    {
        return n1 * t * t;
    }
    else if (t < 2.0f / d1)
    {
        return n1 * (t -= 1.5f / d1) * t + 0.75f;
    }
    else if (t < 2.5f / d1)
    {
        return n1 * (t -= 2.25f / d1) * t + 0.9375f;
    }
    else
    {
        return n1 * (t -= 2.625f / d1) * t + 0.984375f;
    }


}

float Easing::EaseInOutBounce(float t)
{

    if (t < 0.5f)
    {
        EaseOutBounce(1.0f - 2.0f * t);
        return (1.0f - t) / 2.0f;
    }
    else
    {
        EaseOutBounce(2.0f * t - 1.0f);
        return (1.0f + t) / 2.0f;
    }

}
