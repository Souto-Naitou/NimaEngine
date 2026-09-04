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

    inline void ClearTween()
    {
        tweens_.clear();
    }

    void Start(ValueType initValue = {})
    {
        if (!currentTime_) return;
        currentTime_->Reset();
        currentTime_->Start();
        currentValue_ = initValue;
        isPlaying_ = true;
    }

    const ValueType& Update();

    inline void ImGui(const std::string& name = "Timeline")
    {
        #ifdef _DEBUG

        if (ImGui::TreeNode(name.c_str()))
        {
            ImGui::Indent(15.0f);

            if (ImGui::Button("Play")) this->Start();
            ImGui::SameLine();
            if (ImGui::Button("Add")) this->AddTween(0.0f, 1.0f, currentValue_, currentValue_);

            uint32_t index = 0;
            for (auto it = tweens_.begin(); it != tweens_.end();)
            {
                ImGui::PushID(index);

                bool isErase = false;
                ImGui::Separator();
                if (ImGui::Button("Delete"))
                {
                    it = tweens_.erase(it);
                    isErase = true;
                }
                else
                {
                    ImGui::SameLine();
                    it->ImGui("Tween " + std::to_string(index));
                }
                ++index;
                ImGui::Separator();

                if (!isErase) ++it;

                ImGui::PopID();
            }

            ImGui::Unindent(15.0f);
            ImGui::TreePop();
        }



        #endif // _DEBUG
    }

    bool IsPlaying() const { return isPlaying_; }

private:
    std::unique_ptr<TimeMeasurer> currentTime_ = {};
    std::vector<AnimationTween<ValueType>> tweens_ = {};
    ValueType currentValue_ = {};
    bool isPlaying_ = false;
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

    if (!tweens_.empty()) isPlaying_ = !tweens_.back().IsFinished(time);

    return currentValue_;
}