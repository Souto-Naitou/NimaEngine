#pragma once

#include "TransBase.h"
#include <memory>
#include <string>
#include <Core/DirectX12/DirectX12.h>
#include <Features/Layer/Canvas.h>
#include <Features/Layer/OrderedCanvasLayer.h>
#include <optional>
#include <Features/screen/IntermediateScreenBase.h>
#include <Interfaces/IIntermediateScreenFactory.h>
#include <unordered_map>
#include <cstdint>
#include <map>

class STEUpdateWithoutLoading;
class STEUpdateWithLoading;

/// <summary>
/// シーン遷移管理クラス
/// </summary>
class SceneTransitionExecutor
{
public:
    enum class State : uint32_t
    {
        Ready,
        CreateLoadingScreen,
        PlayLoadingScreen,
        ChangeScene,
        End,
    };

    struct Context
    {
        std::optional<std::string>              loadingScreenName_      = std::nullopt;
        std::string                             nextSceneName_          = {};
        std::unique_ptr<IntermediateScreenBase> pInterScreen_           = nullptr;
        std::unique_ptr<TransBase>              pTransition_            = nullptr;
        IIntermediateScreenFactory*             pInterScreenFactory_    = nullptr;
        ISceneArgs*                             pArg_                   = nullptr;
        bool                                    isScreenInitialized_    = false;
    };

    SceneTransitionExecutor() = default;
    ~SceneTransitionExecutor() = default;

    void Initialize(const Canvas::Params& canvasParams, OrderedCanvasLayer* pLayer);
    void Finalize();

    /// <summary>
    /// 遷移エフェクトを実行します。遷移エフェクトがシーンシーンマネージャを通じてシーンの切り替えを行います。
    /// </summary>
    /// <param name="_sceneName">次のシーン名</param>
    /// <param name="_scenePtr">トランジションインスタンス</param>
    void Run(const std::string& sceneName, std::unique_ptr<TransBase>&& transition);

    void Run(
        const std::string& sceneName, 
        const std::string& loadingName, 
        std::unique_ptr<TransBase>&& transition,
        ISceneArgs* pArg);

    /// <summary>
    /// 遷移エフェクトの更新を行います。
    /// </summary>
    void Update();

    /// <summary>
    /// 遷移エフェクトの描画を行います。
    /// </summary>
    void Draw();

    void SetIntermediateScreenFactory(IIntermediateScreenFactory* pFactory) { context_.pInterScreenFactory_ = pFactory; }

    Context& GetContext() { return context_; }
    const Context& GetContext() const { return context_; }

private:
    void UpdateWithoutLoading();
    void UpdateWithLoading();

    std::unique_ptr<STEUpdateWithoutLoading> updateWOLoading_   = nullptr;
    std::unique_ptr<STEUpdateWithLoading>    updateWLoading_    = nullptr;
    std::unique_ptr<Canvas> pCanvas_    = nullptr;
    OrderedCanvasLayer*     pLayer_     = nullptr;
    Context                 context_    = {};
};

class STEUpdateWithoutLoading
{
public:
    enum class Phase
    {
        PlayInAnimation,
        ReserveScene,
        PlayOutAnimation,
        End,
    };

    STEUpdateWithoutLoading() noexcept;
    void operator()(SceneTransitionExecutor* pExecutor);
    void Reset() noexcept;

private:
    std::map<Phase, bool> flags_;
};

class STEUpdateWithLoading
{
public:
    enum class Phase
    {
        PlayInAnimation0,
        CreateLoadingScreen,
        PreloadNextScene,
        PlayOutAnimation0,
        UpdateLoadingScreen,
        PlayInAnimation1,
        ReserveScene,
        PlayOutAnimation1,
        End,
    };

    STEUpdateWithLoading() noexcept;
    void operator()(SceneTransitionExecutor* pExecutor);
    void Reset() noexcept;

private:
    bool IsEnablePhase(Phase phase);
    std::map<Phase, bool> flags_;
};