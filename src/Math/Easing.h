// Copyright © 2024 Souto-Naitou. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

namespace Math::Easing
{
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
