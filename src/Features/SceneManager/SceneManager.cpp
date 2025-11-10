#include "SceneManager.h"

#include <imgui.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <Core/ConfigManager/ConfigManager.h>
#include <Scene/Args/SceneArgs.h>

#include <cassert>

void SceneManager::SetSceneFactory(ISceneFactory* _pSceneFactory)
{
    pSceneFactory_ = _pSceneFactory;
}

void SceneManager::SetSceneArgs(std::unique_ptr<ISceneArgs> _pSceneArgs)
{
    pSceneArgs_ = std::move(_pSceneArgs);
}

void SceneManager::SetModelManager(ModelManager* _pModelManager)
{
    pModelManager_ = _pModelManager;
}

SceneManager& SceneManager::AddInitialArg(const std::string& _key, const std::any& _value)
{
    initialArgs_[_key] = _value;
    return *this;
}

void SceneManager::ReserveScene(const std::string& _name)
{
    isReserveScene_ = true;
    nextSceneName_ = _name;
}

void SceneManager::ReserveScene(const std::string& sceneName, std::unique_ptr<TransBase>&& transition)
{
    pTransitionExecuter_->Run(sceneName, std::move(transition));
}

void SceneManager::ReserveStartupScene()
{
    auto& cfgData = ConfigManager::GetInstance()->GetConfigData();
    this->ReserveScene(cfgData.start_scene);
}

void SceneManager::Initialize(const Params& param)
{
    parameters_ = param;

    DebugManager::GetInstance()->SetComponent("Core", name_, std::bind(&SceneManager::ImGui, this), true);
    pSceneArgs_ = std::make_unique<SceneArgs>();

    pTransitionExecuter_ = std::make_unique<SceneTransitionExecuter>();
    Canvas::Params canvasParam = {};
    canvasParam.name = "SceneTransitionCanvas";
    canvasParam.pDx12 = parameters_.pDx12;
    canvasParam.pCubemapSystem = nullptr;
    canvasParam.pImGuiManager = param.pImGuiManager;
    pTransitionExecuter_->Initialize(canvasParam, param.pLayer);

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

    pTransitionExecuter_->Update();
}

void SceneManager::SceneDraw()
{
    if (pCurrentScene_ != nullptr)
    {
        pCurrentScene_->Draw();
    }

    pTransitionExecuter_->Draw();
}

void SceneManager::SceneDrawText()
{
    if (pCurrentScene_ != nullptr)
    {
        pCurrentScene_->DrawTexts();
    }
}

void SceneManager::Finalize()
{
    if (pCurrentScene_ != nullptr)
    {
        pCurrentScene_->Finalize();
    }

    pTransitionExecuter_->Finalize();

    DebugManager::GetInstance()->DeleteComponent("Core", name_);
}

void SceneManager::ChangeScene()
{
    assert(pSceneFactory_);

    if (pCurrentScene_ != nullptr)
    {
        pCurrentScene_->Finalize();
    }

    this->PackSceneArgs();

    pCurrentScene_ = pSceneFactory_->CreateScene(nextSceneName_, pSceneArgs_.get());
    pCurrentScene_->Initialize();
}

void SceneManager::ImGui()
{
#ifdef _DEBUG

    ImGui::InputText("Next Scene Name", buffer, 128);
    ImGui::SameLine();
    if (ImGui::Button("Change"))
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
