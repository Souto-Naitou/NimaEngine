#pragma once
#include <functional>
#include <string>
#include <nlohmann/json.hpp>

#include <imgui.h>
#include <Utility/JSON/jsonutl.h>
#include <DebugTools/ImGuiTemplates/ImGuiTemplates.h>
#include <Math/Easing.h>

// アニメーションの補間を管理するクラス
//   ValueTypeはUpdate関数内で+-*を使用するため、演算子オーバーロードが必要

template <typename ValueType>
class AnimationTween
{
public:
    AnimationTween(float startSec, float durationSec, const ValueType& startValue, const ValueType& targetValue) :
        startSec_(startSec),
        durationSec_(durationSec),
        startValue_(startValue),
        targetValue_(targetValue)
    {};

    ~AnimationTween() = default;

    void Reset()
    {
        isCalledOnFinished_ = false;
    }

    // 補間関数を設定
    inline void SetTransitionFunction(const std::function<float(float)>& func)
    {
        transitionFunction_ = func;
        easingType_ = Math::Easing::EasingType::None;
    }

    // 補間関数を設定 (EasingTypeを指定)
    inline void SetTransitionFunction(Math::Easing::EasingType type)
    {
        transitionFunction_ = Math::Easing::GetEasingFunction(type);
        easingType_ = type;
    }

    // アニメーション終了時のコールバック関数を設定
    inline void SetOnFinished(const std::function<void()>& func)
    {
        onFinished_ = func;
    }

    // アニメーションが終了したかどうかを判定
    bool IsFinished(float currentTime) const
    {
        return currentTime >= (startSec_ + durationSec_);
    }

    // アニメーションの更新
    void Update(float currentTime, ValueType& currentValue);

    // ImGuiでの表示
    void ImGui(const std::string& name);

    // アニメーションの開始時間を取得
    inline float GetStartSec() const { return startSec_; }

    // アニメーションの継続時間を取得
    inline float GetDurationSec() const { return durationSec_; }

    // アニメーションの開始値を取得
    ValueType GetStartValue() const { return startValue_; }

    // アニメーションの終了値を取得
    ValueType GetTargetValue() const { return targetValue_; }

    // アニメーションの補間タイプを取得
    Math::Easing::EasingType GetEasingType() const { return easingType_; }

private:
    float           startSec_           = 0.0f;
    float           durationSec_        = 0.0f;
    ValueType       targetValue_        = {};
    ValueType       startValue_         = {};
    bool            isCalledOnFinished_ = false;

    // 補間関数
    std::function<float(float)> transitionFunction_ = nullptr;
    // コールバック関数
    std::function<void()> onFinished_ = nullptr;

    // 補間タイプ
    Math::Easing::EasingType easingType_ = Math::Easing::EasingType::None;
};

template<typename ValueType>
inline void AnimationTween<ValueType>::Update(float currentTime, ValueType& currentValue)
{
    // アニメーションが開始していない場合は何もしない
    if (currentTime < startSec_) return;

    // 経過時間に基づいて補間係数を計算
    float t = (currentTime - startSec_) / durationSec_;
    if (t > 1.0f) t = 1.0f;

    // 補間関数が設定されていれば適用
    if (transitionFunction_)
    {
        t = transitionFunction_(t);
    }

    currentValue = startValue_ + (targetValue_ - startValue_) * t; // ここでは単純な線形補間を仮定

    // アニメーションが終了している。かつ、まだコールバックを呼び出していないとき
    // コールバックを呼び出す (ただし続行する)
    if (this->IsFinished(currentTime) && !isCalledOnFinished_)
    {
        onFinished_ ? onFinished_() : void();
        isCalledOnFinished_ = true;
    }
}

template<typename ValueType>
inline void AnimationTween<ValueType>::ImGui(const std::string& name)
{
    #ifdef _DEBUG

    if (ImGui::TreeNode(name.c_str()))
    {
        ImGui::Indent(15.0f);

        ImGui::DragFloat("Start Sec", &startSec_, 0.01f, 0.0f, 100.0f, "%.2f");
        ImGui::DragFloat("Duration Sec", &durationSec_, 0.01f, 0.0f, 100.0f, "%.2f");
        ImGuiTemplate::Drag("Start Value", &startValue_);
        ImGuiTemplate::Drag("Target Value", &targetValue_);

        if (ImGuiTemplate::ComboEnum("Easing Type", easingType_))
        {
            this->SetTransitionFunction(easingType_);
        }

        ImGui::Unindent(15.0f);
        ImGui::TreePop();
    }

    #endif // _DEBUG
}
