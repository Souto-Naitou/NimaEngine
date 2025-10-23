#pragma once

#include "TransBase.h"
#include <memory>
#include <string>

/// <summary>
/// シーン遷移管理クラス
/// </summary>
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

    /// <summary>
    /// 遷移エフェクトを設定し、次のシーン名を予約します。
    /// </summary>
    /// <param name="_sceneName">次のシーン名。</param>
    /// <param name="_scenePtr">適用するシーン遷移の所有権。</param>
    void ChangeScene(const std::string& _sceneName, std::unique_ptr<TransBase>&& _scenePtr);
    /// <summary>
    /// 遷移エフェクトの更新を行います。
    /// </summary>
    void Update();
    /// <summary>
    /// 遷移エフェクトの描画を行います。
    /// </summary>
    void Draw();

private:
    SceneTransitionManager() = default;
    ~SceneTransitionManager() = default;

    std::unique_ptr<TransBase> pTransition_;
};