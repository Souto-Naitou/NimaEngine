#include "DeltaTimeStopWatch.h"
#include <Features/DeltaTimeManager/DeltaTimeManager.h>


void DeltaTimeStopWatch::Start()
{
    isStart_ = true;
    isRunning_ = true;
}

void DeltaTimeStopWatch::Update(uint32_t deltaTimeChannelNum)
{
    if (!isStart_ || !isRunning_) return;
    now_ += DeltaTimeManager::GetInstance()->GetDeltaTime(deltaTimeChannelNum);
}

void DeltaTimeStopWatch::Stop()
{
    isRunning_ = false;
}

void DeltaTimeStopWatch::Reset()
{
    isRunning_ = false;
    isStart_ = false;
    now_ = 0.0;
}
