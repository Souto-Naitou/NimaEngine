#pragma once
#include <string>
#include <Features/TimeMeasurer/TimeMeasurer.h>
#include <Features/Layer/Canvas.h>


/// <summary>
/// シーン遷移の基底クラス
/// </summary>
class TransBase
{
public:
    virtual ~TransBase() {};
    /// <summary>
    /// 遷移の初期化を行います。
    /// </summary>
    /// <param name="sceneName">遷移先のシーン名。</param>
    virtual void Initialize(const std::string& sceneName, Canvas* pCanvas) = 0;
    /// <summary>
    /// 遷移の状態を更新します。
    /// </summary>
    virtual void Update() = 0;
    /// <summary>
    /// 遷移の描画を行います。
    /// </summary>
    virtual void Draw() = 0;
    /// <summary>
    /// 後始末を行います。
    /// </summary>
    virtual void Finalize() = 0;
    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    virtual void ImGui() = 0;

    bool IsEnd() const { return isEnd_; }

protected:
    TimeMeasurer timer_;
    bool isEnd_ = false;

private:
};