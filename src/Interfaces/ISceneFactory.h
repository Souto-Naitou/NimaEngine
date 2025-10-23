#pragma once

#include <Scene/SceneBase.h>
#include <string>

#include <memory>
#include "./ISceneArgs.h"

/// <summary>
/// シーンファクトリ インターフェース
/// </summary>
class ISceneFactory
{
public:
    virtual ~ISceneFactory() = default;
    
    /// <summary>
    /// シーン名と引数に基づいてシーンインスタンスを生成します。
    /// </summary>
    /// <param name="_sceneName">生成するシーン名。</param>
    /// <param name="_pArgs">シーンに渡す引数（所有権は移動しない）。</param>
    /// <returns>生成されたシーン。</returns>
    virtual std::unique_ptr<SceneBase> CreateScene(const std::string& _sceneName, ISceneArgs* _pArgs) = 0;
};