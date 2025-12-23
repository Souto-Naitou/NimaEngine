#include "SceneTransitionExecuter.h"
#include <cassert>

void SceneTransitionExecuter::Initialize(const Canvas::Params& canvasParams, OrderedCanvasLayer* pLayer)
{
    assert(pLayer && "pLayer is nullptr");

    pCanvas_ = std::make_unique<Canvas>();
    pCanvas_->Initialize(canvasParams);
    pLayer_ = pLayer;
    pLayer_->AddCanvas(pCanvas_.get(), 100);
}

void SceneTransitionExecuter::Finalize()
{
    pLayer_->RemoveCanvas(pCanvas_.get());
}

void SceneTransitionExecuter::Run(const std::string& sceneName, std::unique_ptr<TransBase>&& transition)
{
    if (pTransition_) pTransition_->Finalize();
    pTransition_ = std::move(transition);
    pTransition_->Initialize(sceneName, pCanvas_.get());
}

void SceneTransitionExecuter::Update()
{
    if (!pTransition_) return;
    pTransition_->Update();
    if (pTransition_->IsEnd())
    {
        pTransition_->Finalize();
    }
}

void SceneTransitionExecuter::Draw()
{
    if (!pTransition_) return;
    pTransition_->Draw();
}