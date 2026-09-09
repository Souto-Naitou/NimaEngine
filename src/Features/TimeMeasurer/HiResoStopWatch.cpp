#include "./HiResoStopWatch.h"

HiResoStopWatch::HiResoStopWatch()
{
    QueryPerformanceFrequency(&mFreq_);
}

void HiResoStopWatch::Start()
{
    if (isRunning_) return;

    QueryPerformanceCounter(&mStart_);
    isStart_ = true;
    isRunning_ = true;
}

void HiResoStopWatch::Stop()
{
    if (!isRunning_) return;

    GetNow<double>();
    nowBeforeStop_ += now_;
    isRunning_ = false;
}

void HiResoStopWatch::Reset()
{
    mStart_ = {};
    now_ = 0.0;
    nowBeforeStop_ = 0.0;
    isStart_ = false;
    isRunning_ = false;
}

void HiResoStopWatch::Restart()
{
    Reset();
    Start();
}
