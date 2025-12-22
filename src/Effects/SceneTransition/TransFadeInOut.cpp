#include "TransFadeInOut.h"
#include <DebugTools/DebugManager/DebugManager.h>
#include <DebugTools/ImGuiTemplates/ImGuiTemplates.h>
#include <Features/SceneManager/SceneManager.h>
#include <Core/Win32/WinSystem.h>
#include <MathExtension/mathExtension.h>

void TransFadeInOut::Initialize(const std::string& sceneName, Canvas* canvas)
{
    sceneName_ = sceneName;

    sprite_ = std::make_unique<Sprite>();
    sprite_->Initialize("white1x1.png");
    sprite_->SetColor({ 0,0,0,0 });
    sprite_->SetSize({ WinSystem::clientWidth, WinSystem::clientHeight });
    timer_.Start();
    pDebugEntry_ = std::make_unique<DebugEntry<TransFadeInOut>>("Transition", "FadeInOut", this, false);

    pCanvas_ = canvas;
    pCanvas_->RegisterDrawable(sprite_.get());
}

void TransFadeInOut::Update()
{
    if (countPhase_ == 2)
    {
        isEnd_ = true;
        return;
    }
    if (timer_.GetNow<double>() > duration_)
    {
        timer_.Reset();
        timer_.Start();
        countPhase_++;
    }
    if (!isChangedScene_ && countPhase_ == 1)
    {
        isChangedScene_ = true;
        SceneManager::GetInstance()->ReserveScene(sceneName_);
    }

    if (countPhase_ == 0)
    {
        opacity_ = Math::Lerp(0.0f, 1.0f, static_cast<float>(timer_.GetNow<double>() / duration_));
    }
    else if (countPhase_ == 1)
    {
        opacity_ = Math::Lerp(1.0f, 0.0f, static_cast<float>(timer_.GetNow<double>() / duration_));
    }

    sprite_->SetColor(Vector4(0, 0, 0, opacity_));
    sprite_->Update();
}

void TransFadeInOut::Draw()
{
    sprite_->Draw1F();
}

void TransFadeInOut::Finalize()
{
    sprite_->Finalize();
    pCanvas_->UnregisterDrawable(sprite_.get());
}

void TransFadeInOut::ImGui()
{
#ifdef _DEBUG

    auto pFunc = [&]()
    {
        ImGuiTemplate::VariableTableRow("Scene Name", sceneName_);
        ImGuiTemplate::VariableTableRow("Timer", timer_.GetNow<double>());
        ImGuiTemplate::VariableTableRow("Phase", countPhase_);
        ImGuiTemplate::VariableTableRow("Opacity", opacity_);
    };

    ImGuiTemplate::VariableTable("SCT_FadeInOut", pFunc);

#endif
}

TransFadeInOut::~TransFadeInOut()
{
    OutputDebugStringA("deleted TransFadeInOut\n");
}
