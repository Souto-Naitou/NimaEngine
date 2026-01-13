#include "TransFadeInOut.h"
#include <DebugTools/DebugManager/DebugManager.h>
#include <DebugTools/ImGuiTemplates/ImGuiTemplates.h>
#include <Features/SceneManager/SceneManager.h>
#include <Core/Window/Window.h>
#include <MathExtension/mathExtension.h>
#include <cmath>

TransFadeInOut::~TransFadeInOut()
{
    OutputDebugStringA("deleted TransFadeInOut\n");
}

void TransFadeInOut::Initialize()
{
    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("white1x1.png");
    sprite_->SetColor({ 0,0,0,0 });
    sprite_->SetSize({ Window::clientWidth, Window::clientHeight });
    timer_.Start();
    pDebugEntry_ = std::make_unique<DebugEntry<TransFadeInOut>>("Transition", "FadeInOut", this, false);
}

void TransFadeInOut::Update()
{
    if (timer_.GetNow<float>() >= kDuration_)
    {
        phase_ = Phase::End;
        isEnd_ = true;
        return;
    }

    if (phase_ == Phase::FadeIn)
    {
        UpdateFadeInAnimation();
    }
    else if (phase_ == Phase::FadeOut)
    {
        UpdateFadeOutAnimation();
    }

    sprite_->SetColor(Vector4(0, 0, 0, opacity_));
    sprite_->Update();
}

void TransFadeInOut::Draw1F()
{
    sprite_->Draw1F();
}

void TransFadeInOut::Finalize()
{
    sprite_->Finalize();
}

void TransFadeInOut::ImGui()
{
#ifdef _DEBUG

    auto pFunc = [&]()
    {
        ImGuiTemplate::VariableTableRow("Timer", timer_.GetNow<double>());
        ImGuiTemplate::VariableTableRow("Phase", static_cast<uint32_t>(phase_));
        ImGuiTemplate::VariableTableRow("Opacity", opacity_);
    };

    ImGuiTemplate::VariableTable("SCT_FadeInOut", pFunc);

#endif
}

void TransFadeInOut::PlayInAnimation()
{
    timer_.Reset();
    timer_.Start();
    isPlayed_ = true;
    isEnd_ = false;
    phase_ = Phase::FadeIn;
}

void TransFadeInOut::PlayOutAnimation()
{
    timer_.Reset();
    timer_.Start();
    isPlayed_ = true;
    isEnd_ = false;
    phase_ = Phase::FadeOut;
}

void TransFadeInOut::UpdateFadeInAnimation()
{
    opacity_ = std::lerp(0.0f, 1.0f, timer_.GetNow<float>() / kDuration_);
}

void TransFadeInOut::UpdateFadeOutAnimation()
{
    opacity_ = std::lerp(1.0f, 0.0f, timer_.GetNow<float>() / kDuration_);
}
