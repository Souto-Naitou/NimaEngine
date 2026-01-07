#pragma once
#include <Interfaces/ISceneArgs.h>
#include <Features/Layer/OrderedCanvasLayer.h>

/// <summary>
/// シーン基底クラス
/// </summary>
class SceneBase
{
public:
    /// dtor
    SceneBase(ISceneArgs* pArgs);
    virtual ~SceneBase() {}

    /// ロードシーンで実行する処理
    virtual void OnLoadScene() {}

    /// 初期化
    virtual void Initialize() = 0;

    /// 終了
    virtual void Finalize() = 0;

    /// 更新
    virtual void Update() = 0;

    /// 描画
    virtual void Draw() = 0;

protected:
    ISceneArgs*         pArgs_  = nullptr;
    OrderedCanvasLayer* pLayer_ = nullptr;      // !< レイヤー
};