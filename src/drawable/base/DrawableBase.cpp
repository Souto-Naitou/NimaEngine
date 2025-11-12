#include "DrawableBase.h"

#include <Features/Layer/CurrentCanvas.h>
#include <Features/Layer/Canvas.h>  // Canvas型の完全な定義が必要


DrawableBase::~DrawableBase()
{
    if (pCanvasCurrent_)
    {
        pCanvasCurrent_->UnregisterDrawable(this);
    }
}

void DrawableBase::Draw1F()
{
    if (pCanvasCurrent_ != CurrentCanvas::Get())
    {
        if (pCanvasCurrent_) pCanvasCurrent_->UnregisterDrawable(this);
        pCanvasCurrent_ = CurrentCanvas::Get();
        pCanvasCurrent_->RegisterDrawable(this);
    }

    isDrawCalled_ = true;
}
