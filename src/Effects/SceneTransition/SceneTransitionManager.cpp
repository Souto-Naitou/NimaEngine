#include "SceneTransitionManager.h"

void SceneTransitionManager::ChangeScene(const std::string& _sceneName, std::unique_ptr<TransBase>&& _scenePtr)
{
    if (pTransition_) pTransition_->Finalize();
    pTransition_ = std::move(_scenePtr);
    pTransition_->Initialize(_sceneName);
}

void SceneTransitionManager::Update()
{
    if (!pTransition_) return;
    pTransition_->Update();
    if (pTransition_->IsEnd())
    {
        pTransition_->Finalize();
        pTransition_ = nullptr;
    }
}

void SceneTransitionManager::Draw()
{
    if (!pTransition_) return;
    pTransition_->Draw();
}