#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/// <summary>
/// 時間計測クラス
/// </summary>
class TimeMeasurer
{
public:
    TimeMeasurer();
    ~TimeMeasurer() {};

    void Start();
    void Stop();
    void Reset();
    template <typename T>
    T GetNow();

    bool GetIsStart() const { return isStart_; }

private:
    LARGE_INTEGER mFreq_    = {};
    LARGE_INTEGER mStart_   = {};
    double now_             = 0.0;
    double nowBeforeStop_   = 0.0;
    bool isStart_           = false;
    bool isRunning_         = false;
};

template <typename T>
inline T TimeMeasurer::GetNow()
{
    // 停止しているとき
    if (isStart_ && !isRunning_) return static_cast<T>(now_ + nowBeforeStop_);
    if (!isStart_) return T();

    LARGE_INTEGER mNow = {};
    QueryPerformanceCounter(&mNow);

    now_ = static_cast<double>(mNow.QuadPart - mStart_.QuadPart) / static_cast<double>(mFreq_.QuadPart);

    return static_cast<T>(now_ + nowBeforeStop_);
}
