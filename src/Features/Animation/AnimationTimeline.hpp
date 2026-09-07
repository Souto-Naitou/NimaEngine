#pragma once
#include <vector>
#include "./AnimationTween.hpp"
#include <Features/TimeMeasurer/TimeMeasurer.h>
#include <algorithm>

/// <summary>
/// タイムラインアニメーションクラス
/// </summary>
/// <typeparam name="ValueType">動きをつけたいデータの型</typeparam>
template <typename ValueType>
class AnimationTimeline
{
public:
    AnimationTimeline() = default;
    ~AnimationTimeline() = default;

    // Tweenを追加
    inline void AddTween(const AnimationTween<ValueType>& tween)
    {
        tweens_.emplace_back(tween);
    }

    // Tweenを追加
    inline void AddTween(float startSec, float durationSec, const ValueType& startValue, const ValueType& targetValue)
    {
        tweens_.emplace_back(startSec, durationSec, startValue, targetValue);
    }

    inline void ClearTween()
    {
        tweens_.clear();
    }

    void Start(ValueType initValue = {})
    {
        currentTime_.Reset();
        currentTime_.Start();
        currentValue_ = initValue;
        isPlaying_ = true;
    }

    const ValueType& Update();

    inline void ImGui(const std::string& name = "Timeline")
    {

    }

    bool IsPlaying() const { return isPlaying_; }

    // タイムラインの総時間を取得
    float GetTimelineDurationSec();

    // 
    ValueType GetTargetValueLast() const;

    // Tweenのリストを取得
    std::vector<AnimationTween<ValueType>>& GetTweens() { return tweens_; }
    const std::vector<AnimationTween<ValueType>>& GetTweens() const { return tweens_; }

private:
    TimeMeasurer currentTime_ = {};
    std::vector<AnimationTween<ValueType>> tweens_ = {};
    ValueType currentValue_ = {};
    bool isPlaying_ = false;
};

template <typename ValueType>
ValueType AnimationTimeline<ValueType>::GetTargetValueLast() const
{
    if (tweens_.empty()) return ValueType();
    return tweens_.back().GetTargetValue();
}

template <typename ValueType>
float AnimationTimeline<ValueType>::GetTimelineDurationSec()
{
    float duration = 0.0f;
    for (const auto& tween : tweens_)
    {
        duration = std::max(duration, tween.GetStartSec() + tween.GetDurationSec());
    }
    return duration;
}

template<typename ValueType>
inline const ValueType& AnimationTimeline<ValueType>::Update()
{
    float time = currentTime_.GetNow<float>();
    for (auto& tween : tweens_)
    {
        tween.Update(time, currentValue_);
        if (!tween.IsFinished(time) && tween.GetStartSec() < time)
        {
            break;
        }
    }

    if (!tweens_.empty()) isPlaying_ = !tweens_.back().IsFinished(time);

    return currentValue_;
}