#pragma once
#include "AnimationTimeline.hpp"
#include "AnimationTween.hpp"
#include <nlohmann/json.hpp>
#include <Utility/JSON/jsonutl.h>
#include <Math/Easing.h>
#include <Utility/JSONIO/JSONIO.h>
#include <string>
#include <string_view>
#include <DebugTools/Logger/Logger.h>
#include <stdexcept>

class AnimationSerializer
{
public:
    template <typename ValueType>
    static AnimationTimeline<ValueType> Load(std::string_view path);

    template <typename ValueType>
    static AnimationTimeline<ValueType> FromJson(const nlohmann::json& j);

    template <typename ValueType>
    static void Save(std::string_view path, const AnimationTimeline<ValueType>& timeline);

    template <typename ValueType>
    static nlohmann::json ToJson(const AnimationTimeline<ValueType>& timeline);

};

template <typename ValueType>
AnimationTimeline<ValueType>
AnimationSerializer::Load(std::string_view path)
{
    try
    {
        // JSONIOを使用してJSONファイルをロード
        const auto& j = JSONIO::GetInstance()->Load(std::string(path));
        return FromJson<ValueType>(j);
    }
    catch (const std::exception& e)
    {
        // パスを含むエラーメッセージをログに出力
        LOG_ERROR("Failed to load '" + std::string(path) + "': " + e.what());
        return AnimationTimeline<ValueType>();
    }
}

template <typename ValueType>
AnimationTimeline<ValueType>
AnimationSerializer::FromJson(const nlohmann::json& j)
{
    AnimationTimeline<ValueType> timeline;
    
    auto it = j.find("tweens");
    if (it == j.end() || !it->is_array())
    {
        LOG_ERROR("'tweens' key not found or is not an array in JSON.");
        return timeline; // 空のタイムラインを返す
    }

    for (const auto& jTween : *it)
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
        animationTween.SetTransitionFunction(static_cast<Math::Easing::EasingType>(easingType));
        timeline.AddTween(animationTween);
    }

    return timeline;
}

template <typename ValueType>
nlohmann::json AnimationSerializer::ToJson(const AnimationTimeline<ValueType>& timeline)
{
    nlohmann::json j;
    auto& j_tweens = j["tweens"];

    j_tweens = nlohmann::json::array();

    const auto& tweens = timeline.GetTweens();

    for (const auto& tween : tweens)
    {
        nlohmann::json jTween;
        jTween["startSec"] = tween.GetStartSec();
        jTween["durationSec"] = tween.GetDurationSec();
        jTween["startValue"] = tween.GetStartValue();
        jTween["targetValue"] = tween.GetTargetValue();
        jTween["easingType"] = static_cast<uint32_t>(tween.GetEasingType());
        j_tweens.push_back(jTween);
    }

    return j;
}

template <typename ValueType>
void AnimationSerializer::Save(std::string_view path, const AnimationTimeline<ValueType>& timeline)
{
    // AnimationTimelineをJSONへ変換し、JSONIO経由で保存
    JSONIO::GetInstance()->Save(std::string(path), ToJson<ValueType>(timeline));
}
