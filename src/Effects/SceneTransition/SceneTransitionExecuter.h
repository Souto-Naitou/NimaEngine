#pragma once

#include "TransBase.h"
#include <memory>
#include <string>
#include <Core/DirectX12/DirectX12.h>
#include <Features/Layer/Canvas.h>
#include <Features/Layer/Layer.h>

/// <summary>
/// シーン遷移管理クラス
/// </summary>
class SceneTransitionExecuter
{
public:
    SceneTransitionExecuter() = default;
    ~SceneTransitionExecuter() = default;

    void Initialize(Canvas::Params canvasParams, Layer* pLayer);
    void Finalize();

    /// <summary>
    /// 遷移エフェクトを実行します。遷移エフェクトがシーンシーンマネージャを通じてシーンの切り替えを行います。
    /// </summary>
    /// <param name="_sceneName">次のシーン名</param>
    /// <param name="_scenePtr">トランジションインスタンス</param>
    void Run(const std::string& sceneName, std::unique_ptr<TransBase>&& transition);

    /// <summary>
    /// 遷移エフェクトの更新を行います。
    /// </summary>
    void Update();
    /// <summary>
    /// 遷移エフェクトの描画を行います。
    /// </summary>
    void Draw();

private:
    std::unique_ptr<TransBase> pTransition_;
    std::unique_ptr<Canvas> pCanvas_;
    Layer* pLayer_ = nullptr;
};