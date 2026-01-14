#include "SceneTransitionExecutor.h"
#include <Features/Layer/CanvasScope.h>
#include <cassert>

void SceneTransitionExecutor::Initialize(const Canvas::Params& canvasParams, OrderedCanvasLayer* pLayer)
{
    assert(pLayer && "pLayer is nullptr");

    pCanvas_ = std::make_unique<Canvas>();
    pCanvas_->Initialize(canvasParams);
    pLayer_ = pLayer;
    pLayer_->AddCanvasTop(pCanvas_.get());

    updateWOLoading_ = std::make_unique<STEUpdateWithoutLoading>();
    updateWLoading_ = std::make_unique<STEUpdateWithLoading>();
}

void SceneTransitionExecutor::Finalize()
{
    pLayer_->RemoveCanvas(pCanvas_.get());
}

void SceneTransitionExecutor::Run(const std::string& sceneName, std::unique_ptr<TransBase>&& transition)
{
    auto& pTrans = context_.pTransition_;
    if (pTrans) pTrans->Finalize();
    pTrans = std::move(transition);
    pTrans->Initialize();
    context_.isScreenInitialized_ = false;
    context_.nextSceneName_ = sceneName;
    context_.loadingScreenName_ = std::nullopt;
    context_.pArg_ = nullptr;
    updateWOLoading_->Reset();
}

void SceneTransitionExecutor::Run(
    const std::string& sceneName, 
    const std::string& loadingName, 
    std::unique_ptr<TransBase>&& transition,
    ISceneArgs* pArg)
{
    auto& pTrans = context_.pTransition_;
    if (pTrans) pTrans->Finalize();
    pTrans = std::move(transition);
    pTrans->Initialize();
    context_.isScreenInitialized_ = false;
    context_.nextSceneName_ = sceneName;
    context_.loadingScreenName_ = loadingName;
    context_.pArg_ = pArg;
    updateWLoading_->Reset();
}

void SceneTransitionExecutor::Update()
{
    auto& pTrans = context_.pTransition_;
    if (!pTrans) return;

    /// ローディングスクリーンが必要な場合
    if (context_.loadingScreenName_.has_value())
    {
        this->UpdateWithLoading();
    }
    else
    {
        this->UpdateWithoutLoading();
    }

    if (pTrans) pTrans->Update();
}

void SceneTransitionExecutor::Draw()
{
    if (context_.pTransition_)
    {
        CanvasScope canvasScope(pCanvas_.get());
        context_.pTransition_->Draw1F();
    }

    if (context_.isScreenInitialized_ && context_.pInterScreen_)
    {
        context_.pInterScreen_->Draw();
    }
}

void SceneTransitionExecutor::UpdateWithoutLoading()
{
    (*updateWOLoading_)(this);
}

void SceneTransitionExecutor::UpdateWithLoading()
{
    (*updateWLoading_)(this);
}

