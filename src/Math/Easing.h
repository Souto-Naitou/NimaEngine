// Copyright © 2024 Souto-Naitou. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once
#include <functional>

namespace Math::Easing
{
    enum class EasingType
    {
        None,
        InSine,
        OutSine,
        InOutSine,
        InQuad,
        OutQuad,
        InOutQuad,
        InCubic,
        OutCubic,
        InOutCubic,
        InQuart,
        OutQuart,
        InOutQuart,
        InBack,
        OutBack,
        InOutBack,
        InElastic,
        OutElastic,
        InOutElastic,
        InBounce,
        OutBounce,
        InOutBounce,
        COUNT
    };

    constexpr const char* ToString(EasingType type)
    {
        switch (type)
        {
        case EasingType::None:          return "None";
        case EasingType::InSine:        return "InSine";
        case EasingType::OutSine:       return "OutSine";
        case EasingType::InOutSine:     return "InOutSine";
        case EasingType::InQuad:        return "InQuad";
        case EasingType::OutQuad:       return "OutQuad";
        case EasingType::InOutQuad:     return "InOutQuad";
        case EasingType::InCubic:       return "InCubic";
        case EasingType::OutCubic:      return "OutCubic";
        case EasingType::InOutCubic:    return "InOutCubic";
        case EasingType::InQuart:       return "InQuart";
        case EasingType::OutQuart:      return "OutQuart";
        case EasingType::InOutQuart:    return "InOutQuart";
        case EasingType::InBack:        return "InBack";
        case EasingType::OutBack:       return "OutBack";
        case EasingType::InOutBack:     return "InOutBack";
        case EasingType::InElastic:     return "InElastic";
        case EasingType::OutElastic:    return "OutElastic";
        case EasingType::InOutElastic:  return "InOutElastic";
        case EasingType::InBounce:      return "InBounce";
        case EasingType::OutBounce:     return "OutBounce";
        case EasingType::InOutBounce:   return "InOutBounce";
        default: return "";
        }
    }

    std::function<float(float)> GetEasingFunction(EasingType type);

    /// イージング関数郡
    float EaseNone(float t);

    float EaseInSine(float t);
    float EaseOutSine(float t);
    float EaseInOutSine(float t);

    float EaseInQuad(float t);
    float EaseOutQuad(float t);
    float EaseInOutQuad(float t);

    float EaseInCubic(float t);
    float EaseOutCubic(float t);
    float EaseInOutCubic(float t);

    float EaseInQuart(float t);
    float EaseOutQuart(float t);
    float EaseInOutQuart(float t);

    float EaseInBack(float t);
    float EaseOutBack(float t);
    float EaseInOutBack(float t);

    float EaseInElastic(float t);
    float EaseOutElastic(float t);
    float EaseInOutElastic(float t);

    float EaseInBounce(float t);
    float EaseOutBounce(float t);
    float EaseInOutBounce(float t);
}
