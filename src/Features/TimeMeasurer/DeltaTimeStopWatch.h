#pragma once

#include <cstdint>

/// <summary>
/// 時間計測クラス
/// </summary>
class DeltaTimeStopWatch
{
public:
    DeltaTimeStopWatch() = default;
    ~DeltaTimeStopWatch() = default;
    /// <summary>
    /// 計測を開始します。
    /// </summary>
    void Start();

    /// <summary>
    /// 指定されたデルタタイムチャネルに基づいて内部状態を更新します。
    /// </summary>
    /// <param name="deltaTimeChannelNum">更新に使用するデルタタイムチャネルの番号（インデックス）。どのチャネルの経過時間を用いて更新処理を行うかを指定します。</param>
    void Update(uint32_t deltaTimeChannelNum);

    /// <summary>
    /// 計測を一時停止します。
    /// </summary>
    void Stop();

    /// <summary>
    /// 計測値をリセットします。
    /// </summary>
    void Reset();

    template <typename T>
    /// <summary>
    /// 開始からの経過時間を取得します。
    /// </summary>
    /// <returns>テンプレート型Tに変換された経過時間。</returns>
    T GetNow();

    bool GetIsStart() const { return isStart_; }

private:
    double now_ = 0.0;
    double nowBeforeStop_ = 0.0;
    bool isStart_ = false;
    bool isRunning_ = false;
};

template <typename T>
inline T DeltaTimeStopWatch::GetNow()
{
    return static_cast<T>(now_);
}
