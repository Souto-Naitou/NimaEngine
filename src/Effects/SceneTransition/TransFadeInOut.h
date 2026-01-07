#pragma once

#include <Effects/SceneTransition/TransBase.h>
#include <string>
#include <drawable/sprite/Sprite.h>
#include "DebugTools/DebugEntry/DebugEntry.h"

/// <summary>
/// フェードイン・フェードアウト
/// </summary>
class TransFadeInOut : public TransBase
{
public:
    ~TransFadeInOut();

    /// <summary>
    /// フェードイン・アウトの初期化を行います。
    /// </summary>
    /// <param name="sceneName">遷移先のシーン名。</param>
    void Initialize() override;

    /// <summary>
    /// 状態を更新します。
    /// </summary>
    void Update() override;

    /// <summary>
    /// エフェクトを描画します。
    /// </summary>
    void Draw1F() override;

    /// <summary>
    /// 後始末を行います。
    /// </summary>
    void Finalize() override;

    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void ImGui() override;

    /// <summary>
    /// フェードインを再生します。
    /// </summary>
    void PlayInAnimation() override;

    /// <summary>
    /// フェードアウトを再生します。
    /// </summary>
    void PlayOutAnimation() override;

private:
    enum class Phase : uint32_t
    {
        FadeIn = 0,
        FadeOut = 1,
        End = 2,
    } phase_ = Phase::End;

    void UpdateFadeInAnimation();
    void UpdateFadeOutAnimation();

    static constexpr float kDuration_ = 1.0f;

    std::unique_ptr<DebugEntry<TransFadeInOut>> pDebugEntry_ = nullptr;
    std::unique_ptr<Sprite> sprite_ = nullptr;
    float   opacity_        = 0.0f;
};