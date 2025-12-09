#include "GaugeBase.h"



void GaugeBase::operator=(float val)
{
    rawValue_ = val;
}

float& GaugeBase::operator+=(const float& val)
{
    rawValue_ += val;
    return rawValue_;
}
