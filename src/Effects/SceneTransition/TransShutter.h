#pragma once

#include <Effects/SceneTransition/TransBase.h>
#include <string>
#include <Features/Sprite/Sprite.h>
#include <Features/Animation/AnimationTimeline.h>
#include <Core/Win32/WinSystem.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <memory>


class TransShutter : public TransBase
{
public:
    ~TransShutter() = default;
    void Initialize(const std::string& _sceneName) override;
    void Update() override;
    void Draw() override;
    void Finalize() override {};
    void ImGui() override;

private:
    const float kHalfHeight = WinSystem::clientHeight / 2.0f;


    std::string name_;
    bool        isChangedScene_ = false;
    float       duration_ = 0.25f;


    std::string sceneName_;
    std::unique_ptr<Sprite> spriteUpper_;
    std::unique_ptr<Sprite> spriteLower_;

    AnimationTimeline<float> animDeltaY_ = {};

    std::unique_ptr<DebugEntry<TransShutter>> pDebugEntry_ = nullptr;  //< デバッグエントリ

    void AnimationInitialize();
    void SpriteInitialize();
};