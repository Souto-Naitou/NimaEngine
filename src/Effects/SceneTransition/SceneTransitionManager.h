#pragma once

#include "TransBase.h"
#include <memory>
#include <string>

class SceneTransitionManager
{
public:
    SceneTransitionManager(SceneTransitionManager&) = delete;
    SceneTransitionManager& operator=(SceneTransitionManager&) = delete;
    SceneTransitionManager(SceneTransitionManager&&) = delete;
    SceneTransitionManager& operator=(SceneTransitionManager&&) = delete;

    static SceneTransitionManager* GetInstance()
    {
        static SceneTransitionManager instance;
        return &instance;
    }

    void ChangeScene(const std::string& _sceneName, std::unique_ptr<TransBase>&& _scenePtr);
    void Update();
    void Draw();

private:
    SceneTransitionManager() = default;
    ~SceneTransitionManager() = default;

    std::unique_ptr<TransBase> pTransition_;
};