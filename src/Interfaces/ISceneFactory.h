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
    /// <param name="sceneName">生成するシーン名。</param>
    /// <param name="pArgs">シーンに渡す引数（所有権は移動しない）。</param>
    /// <returns>生成されたシーン。</returns>
    virtual std::unique_ptr<SceneBase> Create(const std::string& sceneName, ISceneArgs* pArgs) = 0;

    /// <summary>
    /// 指定された名前と引数を使用して、読み込み可能なシーンを作成します。
    /// </summary>
    /// <param name="sceneName">作成するシーンの名前。</param>
    /// <param name="pArgs">シーンの作成に使用する引数へのポインタ。</param>
    /// <returns>作成された読み込み可能なシーンへの unique_ptr。</returns>
    virtual std::unique_ptr<ILoadableScene> CreateLoadable(const std::string& sceneName, ISceneArgs* pArgs) = 0;
};