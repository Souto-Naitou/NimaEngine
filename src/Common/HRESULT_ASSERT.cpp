#include "HRESULT_ASSERT.h"

#include <cassert>

HRESULT_ASSERT HRESULT_ASSERT::operator=(const HRESULT& lValue)
{
    hr_ = lValue;
    if (FAILED(hr_))
    {
        assert(false && "HRESULT ERROR");
    }
    return *this;
}

HRESULT_ASSERT HRESULT_ASSERT::operator=(const HRESULT&& rValue)
{
    hr_ = rValue;
    if (FAILED(hr_))
    {
        assert(false && "HRESULT ERROR");
    }
    return *this;
}
