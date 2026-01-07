#pragma once
#include <Scene/SceneBase.h>
#include <io/loader/TaskExecutor.h>
#include <Interfaces/ISceneArgs.h>
#include <Features/Layer/OrderedCanvasLayer.h>

class IntermediateScreenBase
{
public:
    IntermediateScreenBase(ISceneArgs* args) : pArgs_(args) 
    {
        pLayer_ = std::any_cast<OrderedCanvasLayer*>(pArgs_->Get("Layer"));
    };

    virtual ~IntermediateScreenBase(){};

    /// <summary>
    /// 中間スクリーンの初期化を行います。
    /// </summary>
    virtual void Initialize() = 0;

    /// <summary>
    /// 中間スクリーンの更新を行います。
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// 中間スクリーンの描画を行います。
    /// </summary>
    virtual void Draw() = 0;

    /// <summary>
    /// オブジェクトの終了処理を実行します。
    /// </summary>
    virtual void Finalize() = 0;

    /// <summary>
    /// Screenを終了可能かどうかを取得します。
    /// </summary>
    /// <returns>Screenを終了可能な場合は true、それ以外の場合は false。</returns>
    virtual bool IsEnd() const = 0;

    TaskExecutor& GetTaskExecutor() { return taskExecutor_; }

protected:
    TaskExecutor taskExecutor_ = {};
    ISceneArgs* pArgs_ = nullptr;
    OrderedCanvasLayer* pLayer_ = nullptr;
};