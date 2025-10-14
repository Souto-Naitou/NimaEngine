#pragma once

#include <Scene/SceneBase.h>
#include <Interfaces/ISceneFactory.h>
#include <Effects/SceneTransition/SceneTransitionManager.h>
#include <Features/Model/ModelManager.h>
#include <memory>
#include <Interfaces/ISceneArgs.h>
#include <unordered_map>

class SceneManager
{
public:
    SceneManager(SceneManager const&) = delete;
    void operator=(SceneManager const&) = delete;
    SceneManager(SceneManager&&) = delete;
    void operator=(SceneManager&&) = delete;

    static SceneManager* GetInstance()
    {
        static SceneManager instance;
        return &instance;
    }


public: /// Setter
    void SetSceneFactory(ISceneFactory* _pSceneFactory);
    void SetSceneArgs(std::unique_ptr<ISceneArgs> _pSceneArgs);
    void SetModelManager(ModelManager* _pModelManager);

    // すべてのシーンに渡す初期引数を追加
    //   - 値はコピーされるため所有権の移動はできない
    //   - キーはユニークであることが保証されるため、同一キー指定時は上書きされる
    SceneManager& AddInitialArg(const std::string& _key, const std::any& _value);


public:
    void ReserveScene(const std::string& _sceneName);
    void ReserveStartupScene();


public: /// シーン動作
    void Initialize();
    void Update();
    void SceneDraw();
    void SceneDrawText();
    void Finalize();


private:
    SceneManager() = default;
    void ChangeScene();
    void ImGui();
    void PackSceneArgs();

    std::string name_ = "SceneManager";
    char buffer[128] = {};

    bool isReserveScene_ = false;

    std::string nextSceneName_;
    std::unique_ptr<SceneBase> pCurrentScene_ = nullptr;
    std::unique_ptr<ISceneArgs> pSceneArgs_ = nullptr;
    std::unordered_map<std::string, std::any> initialArgs_;


private: /// 他クラスのインスタンス
    ISceneFactory* pSceneFactory_ = nullptr;
    SceneTransitionManager* pSceneTransitionManager_ = nullptr;

    ModelManager* pModelManager_ = nullptr;
};