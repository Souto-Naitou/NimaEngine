#pragma once

#include <Effects/SceneTransition/TransBase.h>
#include <string>
#include <drawable/sprite/Sprite.h>
#include <Features/Animation/AnimationTween.hpp>
#include <Core/Window/Window.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <memory>
#include <array>
#include <Features/Layer/Canvas.h>


/// <summary>
/// シャッターのような上下から閉じるシーン遷移
/// </summary>
class TransShutter : public TransBase
{
public:
    ~TransShutter() = default;
    /// <summary>
    /// シャッター遷移の初期化を行います。
    /// </summary>
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
    /// シャッターを下ろします。
    /// </summary>
    void PlayInAnimation() override;

    /// <summary>
    /// シャッターを上げます。
    /// </summary>
    void PlayOutAnimation() override;

private:
    enum class Phase : uint32_t
    {
        ShutterIn = 0,
        ShutterOut = 1,
        End = 2,
    } phase_ = Phase::End;

    static constexpr float kDuration_ = 2.0f;

    std::unique_ptr<Sprite> pSpriteUpper_ = nullptr;
    std::unique_ptr<Sprite> pSpriteLower_ = nullptr;
    std::unique_ptr<AnimationTween<float>> pTweenIn_ = nullptr;
    std::unique_ptr<AnimationTween<float>> pTweenOut_ = nullptr;
    std::unique_ptr<DebugEntry<TransShutter>> pDebugEntry_ = nullptr;  //< デバッグエントリ

    /// <summary>
    /// アニメーションの初期化を行います。
    /// </summary>
    void AnimationInitialize();
    /// <summary>
    /// 使用するスプライトの生成および初期化を行います。
    /// </summary>
    void SpriteInitialize();
};