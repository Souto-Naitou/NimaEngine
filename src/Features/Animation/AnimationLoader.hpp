#pragma once
#include "AnimationTimeline.hpp"
#include "AnimationTween.hpp"
#include <nlohmann/json.hpp>
#include <Utility/JSON/jsonutl.h>
#include <Math/Easing.h>
#include <Utility/JSONIO/JSONIO.h>
#include <string>

class AnimationLoader
{
public:
    template <typename ValueType>
    static AnimationTimeline<ValueType> Load(const std::string& path);

    template <typename ValueType>
    static AnimationTimeline<ValueType> LoadFromJson(const nlohmann::json& j);
};

template <typename ValueType>
AnimationTimeline<ValueType>
AnimationLoader::Load(const std::string& path)
{
    // JSONIOを使用してJSONファイルをロード
    const auto& j = JSONIO::GetInstance()->Load(path);

    // JSONからAnimationTimelineを生成
    return LoadFromJson<ValueType>(j);
}

template <typename ValueType>
AnimationTimeline<ValueType>
AnimationLoader::LoadFromJson(const nlohmann::json& j)
{
    AnimationTimeline<ValueType> timeline;
    for (const auto& jTween : j.at("tweens"))
    {
        float start, duration;
        ValueType startV, targetV;
        uint32_t easingType;

        utl::json::try_assign(jTween, "startSec", start);
        utl::json::try_assign(jTween, "durationSec", duration);
        utl::json::try_assign(jTween, "startValue", startV);
        utl::json::try_assign(jTween, "targetValue", targetV);
        utl::json::try_assign(jTween, "easingType", easingType);

        auto animationTween = AnimationTween<ValueType>(start, duration, startV, targetV);
        animationTween.SetTransitionFunction(static_cast<Easing::EasingType>(easingType));
        timeline.AddTween(animationTween);
    }

    return timeline;
}
