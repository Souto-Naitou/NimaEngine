// Copyright © 2024 Souto-Naitou. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

namespace Math::Easing
{
    /// イージング関数郡
    float EaseNone(float _t);

    float EaseInSine(float _t);
    float EaseOutSine(float _t);
    float EaseInOutSine(float _t);

    float EaseInQuad(float _t);
    float EaseOutQuad(float _t);
    float EaseInOutQuad(float _t);

    float EaseInCubic(float _t);
    float EaseOutCubic(float _t);
    float EaseInOutCubic(float _t);

    float EaseInQuart(float _t);
    float EaseOutQuart(float _t);
    float EaseInOutQuart(float _t);

    float EaseInBack(float _t);
    float EaseOutBack(float _t);
    float EaseInOutBack(float _t);

    float EaseInElastic(float _t);
    float EaseOutElastic(float _t);
    float EaseInOutElastic(float _t);

    float EaseInBounce(float _t);
    float EaseOutBounce(float _t);
    float EaseInOutBounce(float _t);
}
