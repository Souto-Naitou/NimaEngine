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
    virtual void Initialize() = 0;

    /// <summary>
    /// 遷移の状態を更新します。
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// 遷移の描画を行います。
    /// </summary>
    virtual void Draw1F() = 0;

    /// <summary>
    /// 後始末を行います。
    /// </summary>
    virtual void Finalize() = 0;

    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    virtual void ImGui() = 0;

    /// <summary>
    /// イン アニメーションを再生します。
    /// </summary>
    virtual void PlayInAnimation() = 0;

    /// <summary>
    /// アウトアニメーションを再生します。
    /// </summary>
    virtual void PlayOutAnimation() = 0;

    bool IsEnd() const { return isEnd_; }
    bool IsPlayed() const { return isPlayed_; }
    void Reset() { isEnd_ = false; isPlayed_ = false; timer_.Reset(); }

protected:
    TimeMeasurer timer_;
    bool isEnd_ = false;
    bool isPlayed_ = false;
};