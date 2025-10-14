#include "TransShutter.h"
#include <Features/SceneManager/SceneManager.h>

void TransShutter::Initialize(const std::string& _sceneName)
{
    this->AnimationInitialize();
    this->SpriteInitialize();

    pDebugEntry_ = std::make_unique<DebugEntry<TransShutter>>("Transition", "Shutter", this);

    sceneName_ = _sceneName;

    animDeltaY_.Start();
}

void TransShutter::Update()
{
    float deltaY = animDeltaY_.Update();
    spriteUpper_->SetPosition({ 0.0f, deltaY });
    spriteLower_->SetPosition({ 0.0f, WinSystem::clientHeight - deltaY });
    spriteUpper_->Update();
    spriteLower_->Update();
}

void TransShutter::Draw()
{
    spriteUpper_->Draw();
    spriteLower_->Draw();
}

void TransShutter::ImGui()
{
    #ifdef _DEBUG

    animDeltaY_.ImGui("Shutter");

    #endif // _DEBUG
}

void TransShutter::AnimationInitialize()
{
    float halfDuration = duration_ / 2.0f;

    AnimationTween<float> tweenFirstHalf(0.0f, halfDuration, 0.0f, kHalfHeight);
    AnimationTween<float> tweenSecondHalf(halfDuration + 0.02f, halfDuration, kHalfHeight, 0.0f);

    // シーン切り替え用のコールバックを設定
    auto changeScene = [this]() {
        if (!isChangedScene_)
        {
            isChangedScene_ = true;
            SceneManager::GetInstance()->ReserveScene(sceneName_);
        }
    };

    // 前半のアニメーションが終了したらシーンを切り替える
    tweenFirstHalf.SetOnFinished(changeScene);

    animDeltaY_.AddTween(tweenFirstHalf);
    animDeltaY_.AddTween(tweenSecondHalf);
}

void TransShutter::SpriteInitialize()
{
    spriteUpper_ = std::make_unique<Sprite>();
    spriteUpper_->Initialize("white1x1.png");
    spriteUpper_->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    spriteUpper_->SetSize({ static_cast<float>(WinSystem::clientWidth), kHalfHeight });
    spriteUpper_->SetAnchorPoint({ 0.0f, 1.0f });
    spriteUpper_->SetPosition({ 0.0f, 0.0f });

    spriteLower_ = std::make_unique<Sprite>();
    spriteLower_->Initialize("white1x1.png");
    spriteLower_->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    spriteLower_->SetSize({ static_cast<float>(WinSystem::clientWidth), kHalfHeight });
    spriteLower_->SetAnchorPoint({ 0.0f, 0.0f });
    spriteLower_->SetPosition({ 0.0f, static_cast<float>(WinSystem::clientHeight) });
}
