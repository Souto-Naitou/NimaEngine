#include "AnimationHelper.h"
#include <cassert>

Vector3 Helper::Animation::CalculateValue(const AnimationCurve<Vector3>& kf, float time)
{
    Vector3 result = {};

    const auto& keyframes = kf.keyframes;

    if (keyframes.empty())
    {
        #ifdef _ENGINE_DEBUG_ANIMATION
        throw std::runtime_error("No keyframes available for interpolation.");
        #endif
        return result;
    }
    if (keyframes.size() == 1 || time <= keyframes.front().time)
    {
        // キーが1つだけ、または時間が最初のキーの時間以下の場合は、最初のキーの値を返す
        result = keyframes.front().value;
        return result;
    }

    for (size_t index = 0; index < keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
        {
            // 現在のキーと次のキーの間に時間がある場合、線形補間を行う
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            result.Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
            return result;
        }
    }

    // 最後のキーの時間を超えた場合は、最後のキーの値を返す
    result = keyframes.back().value;
    return result;
}

Quaternion Helper::Animation::CalculateValue(const AnimationCurve<Quaternion>& kf, float time)
{
    Quaternion result = {};
    const auto& keyframes = kf.keyframes;
    if (keyframes.empty())
    {
        #ifdef _ENGINE_DEBUG_ANIMATION
        throw std::runtime_error("No keyframes available for interpolation.");
        #endif
        return result;
    }
    if (keyframes.size() == 1 || time <= keyframes.front().time)
    {
        // キーが1つだけ、または時間が最初のキーの時間以下の場合は、最初のキーの値を返す
        result = keyframes.front().value;
        return result;
    }
    for (size_t index = 0; index < keyframes.size() - 1; ++index)
    {
        size_t nextIndex = index + 1;
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time)
        {
            // 現在のキーと次のキーの間に時間がある場合、線形補間を行う
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            result = Quaternion::Slerp(keyframes[index].value, keyframes[nextIndex].value, t);

            return result;
        }
    }
    // 最後のキーの時間を超えた場合は、最後のキーの値を返す
    result = keyframes.back().value;
    return result;
}