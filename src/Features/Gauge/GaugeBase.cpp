#include "GaugeBase.h"



void GaugeBase::operator=(const float& val)
{
    targetValue_ = val;
}

float& GaugeBase::operator+=(const float& val)
{
    targetValue_ += val;
    return targetValue_;
}
