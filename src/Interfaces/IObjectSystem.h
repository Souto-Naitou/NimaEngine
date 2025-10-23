#pragma once

class IObjectSystem
{
public:
    // dtor
    virtual ~IObjectSystem() = default;
    /// <summary>
    /// オブジェクトシステムを初期化します。
    /// </summary>
    virtual void Initialize() = 0;
    /// <summary>
    /// 描画前の準備処理を行います（コマンド記録の前処理など）。
    /// </summary>
    virtual void PrepareDraw() = 0;
};