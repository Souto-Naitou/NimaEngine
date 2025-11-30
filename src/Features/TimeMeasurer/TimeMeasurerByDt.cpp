#include "TimeMeasurerByDt.h"
#include <Features/DeltaTimeManager/DeltaTimeManager.h>


void TimeMeasurerByDt::Start()
{
    isStart_ = true;
    isRunning_ = true;
}

void TimeMeasurerByDt::Update(uint32_t deltaTimeChannelNum)
{
    if (!isStart_ || !isRunning_) return;
    now_ += DeltaTimeManager::GetInstance()->GetDeltaTime(deltaTimeChannelNum);
}

void TimeMeasurerByDt::Stop()
{
    isRunning_ = false;
}

void TimeMeasurerByDt::Reset()
{
    isRunning_ = false;
    isStart_ = false;
    now_ = 0.0;
}
