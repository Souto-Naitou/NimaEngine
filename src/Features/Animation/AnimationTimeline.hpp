#pragma once
#include <memory>
#include <vector>
#include "./AnimationTween.hpp"
#include <Features/TimeMeasurer/TimeMeasurer.h>

/// <summary>
/// タイムラインアニメーションクラス
/// </summary>
/// <typeparam name="ValueType">動きをつけたいデータの型</typeparam>
template <typename ValueType>
class AnimationTimeline
{
public:
    inline AnimationTimeline()
    {
        currentTime_ = std::make_unique<TimeMeasurer>();
    }

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

    void Start(ValueType initValue = {})
    {
        if (!currentTime_) return;
        currentTime_->Reset();
        currentTime_->Start();
        currentValue_ = initValue;
    }

    const ValueType& Update();

    inline void ImGui(const std::string& name = "Timeline")
    {
        #ifdef _DEBUG

        if (ImGui::TreeNode(name.c_str()))
        {
            ImGui::Indent(15.0f);

            if (ImGui::Button("Play")) this->Start();

            uint32_t index = 0;
            for (auto& tween : tweens_)
            {
                tween.ImGui("Tween " + std::to_string(index));
                ++index;
            }

            ImGui::Unindent(15.0f);
            ImGui::TreePop();
        }

        #endif // _DEBUG
    }


private:
    std::unique_ptr<TimeMeasurer> currentTime_ = {};
    std::vector<AnimationTween<ValueType>> tweens_ = {};
    ValueType currentValue_ = {};
};

template<typename ValueType>
inline const ValueType& AnimationTimeline<ValueType>::Update()
{
    if (!currentTime_) return currentValue_;

    float time = currentTime_->GetNow<float>();
    for (auto& tween : tweens_)
    {
        tween.Update(time, currentValue_);
        if (!tween.IsFinished(time) && tween.GetStartSec() < time)
        {
            break;
        }
    }

    return currentValue_;
}