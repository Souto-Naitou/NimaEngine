#include "TransShutter.h"
#include <Math/ViewportUnits.hpp>
#include <Math/Easing.h>

using namespace Math::Viewport;

void TransShutter::Initialize()
{
    this->SpriteInitialize();
    this->AnimationInitialize();

    pDebugEntry_ = std::make_unique<DebugEntry<TransShutter>>("Transition", "Shutter", this);
}

void TransShutter::Update()
{
    if (phase_ == Phase::End)
    {
        return;
    }

    const float now = timer_.GetNow<float>();
    float deltaY = 0.0f;
    if (phase_ == Phase::ShutterIn)
    {
        pTweenIn_->Update(now, deltaY);
    }
    else if (phase_ == Phase::ShutterOut)
    {
        pTweenOut_->Update(now, deltaY);
    }

    /// スプライトの位置更新
    pSpriteUpper_->SetPosition({ 0.0f, deltaY });
    pSpriteLower_->SetPosition({ 0.0f, 100.0_vh - deltaY });
    pSpriteUpper_->Update();
    pSpriteLower_->Update();
}

void TransShutter::Draw1F()
{
    pSpriteUpper_->Draw1F();
    pSpriteLower_->Draw1F();
}

void TransShutter::Finalize()
{
}

void TransShutter::ImGui()
{
    #ifdef _DEBUG

    pTweenIn_->ImGui("In");
    pTweenOut_->ImGui("Out");

    #endif // _DEBUG
}

void TransShutter::PlayInAnimation()
{
    phase_ = Phase::ShutterIn;
    isPlayed_ = true;
    isEnd_ = false;
    timer_.Reset();
    timer_.Start();
    pTweenIn_->Reset();
}

void TransShutter::PlayOutAnimation()
{
    phase_ = Phase::ShutterOut;
    isPlayed_ = true;
    isEnd_ = false;
    timer_.Reset();
    timer_.Start();
    pTweenOut_->Reset();
}

void TransShutter::AnimationInitialize()
{
    float halfDuration = kDuration_ / 2.0f;

    pTweenIn_ = std::make_unique<AnimationTween<float>>(0.0f, halfDuration, 0.0f, 50.0_vh);
    pTweenOut_ = std::make_unique<AnimationTween<float>>(0.0f, halfDuration, 50.0_vh, 0.0f);

    // シーン切り替え用のコールバックを設定
    auto finish = [this]()
    {
        isEnd_ = true;
        phase_ = Phase::End;
    };

    pTweenIn_->SetOnFinished(finish);
    pTweenOut_->SetOnFinished(finish);
    pTweenIn_->SetTransitionFunction(Math::Easing::EaseInOutCubic);
    pTweenOut_->SetTransitionFunction(Math::Easing::EaseInOutCubic);
}

void TransShutter::SpriteInitialize()
{
    pSpriteUpper_ = std::make_unique<Sprite>();
    pSpriteUpper_->Initialize("white1x1.png");
    pSpriteUpper_->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    pSpriteUpper_->SetSize({ 100.0_vw, 50.0_vh });
    pSpriteUpper_->SetAnchorPoint({ 0.0f, 1.0f });
    pSpriteUpper_->SetPosition({ 0.0f, 0.0f });

    pSpriteLower_ = std::make_unique<Sprite>();
    pSpriteLower_->Initialize("white1x1.png");
    pSpriteLower_->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
    pSpriteLower_->SetSize({ 100.0_vw, 50.0_vh });
    pSpriteLower_->SetAnchorPoint({ 0.0f, 0.0f });
    pSpriteLower_->SetPosition({ 0.0f, 100.0_vh });
}
