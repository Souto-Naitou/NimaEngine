#include "SceneManager.h"

#include <imgui.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <Core/ConfigManager/ConfigManager.h>
#include <Scene/Args/SceneArgs.h>

#include <cassert>

void SceneManager::SetSceneFactory(ISceneFactory* pSceneFactory)
{
    pSceneFactory_ = pSceneFactory;
}

void SceneManager::SetTransitionExecutor(SceneTransitionExecutor* pTransitionExecutor)
{
    pTransitionExecutor_ = pTransitionExecutor;
}

void SceneManager::SetSceneArgs(std::unique_ptr<ISceneArgs> pSceneArgs)
{
    pSceneArgs_ = std::move(pSceneArgs);
}

void SceneManager::SetModelManager(ModelManager* pModelManager)
{
    pModelManager_ = pModelManager;
}

SceneManager& SceneManager::AddInitialArg(const std::string& key, const std::any& value)
{
    initialArgs_[key] = value;
    return *this;
}

void SceneManager::ReserveScene(const std::string& name)
{
    isReserveScene_ = true;
    nextSceneName_ = name;
}

void SceneManager::ReserveScene(const std::string& sceneName, std::unique_ptr<TransBase>&& transition)
{
    pTransitionExecutor_->Run(sceneName, std::move(transition));
}

void SceneManager::ReserveScene(const std::string& sceneName, const std::string& loadingName, std::unique_ptr<TransBase>&& transition)
{
    this->PackSceneArgs();
    pTransitionExecutor_->Run(sceneName, loadingName, std::move(transition), pSceneArgs_.get());
}

void SceneManager::ReserveStartupScene()
{
    auto& cfgData = ConfigManager::GetInstance()->GetConfigData();
    this->ReserveScene(cfgData.start_scene);
}

void SceneManager::Initialize()
{
    pDebugEntry_ = std::make_unique<DebugEntry<SceneManager>>("Core", name_, this, true);
    pSceneArgs_ = std::make_unique<SceneArgs>();
    this->ReserveStartupScene();
}

void SceneManager::Update()
{
    if (isReserveScene_)
    {
        ChangeScene();
        isReserveScene_ = false;
    }

    if (pCurrentScene_ != nullptr)
    {
        pCurrentScene_->Update();
    }

    pTransitionExecutor_->Update();
}

void SceneManager::SceneDraw()
{
    if (pCurrentScene_ != nullptr)
    {
        pCurrentScene_->Draw();
    }

    pTransitionExecutor_->Draw();
}

void SceneManager::Finalize()
{
    if (pCurrentScene_ != nullptr)
    {
        pCurrentScene_->Finalize();
    }

    pTransitionExecutor_->Finalize();
}

void SceneManager::ScenePreload(const std::string& sceneName, TaskExecutor& taskExec)
{
    assert(pSceneFactory_);
    pPreloadedScene_ = pSceneFactory_->CreateLoadable(sceneName, pSceneArgs_.get());
    pPreloadedScene_->PreLoad(taskExec);
}

void SceneManager::ChangeScene()
{
    assert(pSceneFactory_);

    if (pCurrentScene_ != nullptr)
    {
        pCurrentScene_->Finalize();
    }

    this->PackSceneArgs();

    if (pPreloadedScene_)
    {
        pCurrentScene_ = std::move(pPreloadedScene_);
        pPreloadedScene_ = nullptr;
        pCurrentScene_->Initialize();
        return;
    }
    else
    {
        pCurrentScene_ = pSceneFactory_->Create(nextSceneName_, pSceneArgs_.get());
        pCurrentScene_->Initialize();
    }
}

void SceneManager::ImGui()
{
#ifdef _DEBUG

    bool isConfirmed = false;
    isConfirmed = ImGui::InputText("Next Scene Name", buffer, 128, ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    isConfirmed = ImGui::Button("Change");

    if (isConfirmed)
    {
        this->ReserveScene(buffer);
    }

#endif // _DEBUG
}

void SceneManager::PackSceneArgs()
{
    if (pSceneArgs_ == nullptr)
    {
        pSceneArgs_ = std::make_unique<SceneArgs>();
    }

    for (const auto& [key, value] : initialArgs_)
    {
        pSceneArgs_->Set(key, value);
    }

    pSceneArgs_->Set("ModelManager", pModelManager_);
}
