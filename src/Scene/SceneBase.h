#pragma once
#include <Interfaces/ISceneArgs.h>

/// <summary>
/// シーン基底クラス
/// </summary>
class SceneBase
{
public:
    /// dtor
    SceneBase(ISceneArgs* _pArgs);
    virtual ~SceneBase() {}

    /// 初期化
    virtual void Initialize() = 0;

    /// 終了
    virtual void Finalize() = 0;

    /// 更新
    virtual void Update() = 0;

    /// 描画(テキスト描画を除く)
    ///   - テキスト描画はDrawTexts()で行う
    ///   - 描画前処理も同時に行う。pArgs_から必要なポインタを取得できる
    ///   - pArgsに含まれるデータはNimaFramework::Initialize()を参照
    virtual void Draw() = 0;

    /// テキスト描画
    virtual void DrawTexts() = 0;

protected:
    ISceneArgs* pArgs_ = nullptr;
};