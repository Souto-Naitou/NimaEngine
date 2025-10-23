#pragma once

#include <Effects/SceneTransition/TransBase.h>
#include <string>
#include <Features/Sprite/Sprite.h>
#include <Features/Animation/AnimationTimeline.h>
#include <Core/Win32/WinSystem.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <memory>


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
    /// <param name="_sceneName">遷移先のシーン名。</param>
    void Initialize(const std::string& _sceneName) override;
    /// <summary>
    /// 状態を更新します。
    /// </summary>
    void Update() override;
    /// <summary>
    /// エフェクトを描画します。
    /// </summary>
    void Draw() override;
    /// <summary>
    /// 後始末を行います。
    /// </summary>
    void Finalize() override {};
    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void ImGui() override;

private:
    const float kHalfHeight = WinSystem::clientHeight / 2.0f;


    std::string name_;
    bool        isChangedScene_ = false;
    float       duration_ = 2.0f;


    std::string sceneName_;
    std::unique_ptr<Sprite> spriteUpper_;
    std::unique_ptr<Sprite> spriteLower_;

    AnimationTimeline<float> animDeltaY_ = {};

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