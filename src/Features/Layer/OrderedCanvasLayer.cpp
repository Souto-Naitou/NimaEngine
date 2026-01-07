#include "OrderedCanvasLayer.h"



bool IsCanvasEnabled(Canvas* canvas)
{
    bool isEnabled = canvas->IsEnabled();
    isEnabled &= !canvas->IsEnableManualDraw();
    return isEnabled;
}

void OrderedCanvasLayer::Initialize()
{

}

uint32_t OrderedCanvasLayer::AddCanvas(Canvas* canvas, uint32_t zOrder)
{
    uint32_t assignedZOrder = zOrder;

    if (zOrder == 0)
    {
        assignedZOrder = nextZOrder_;
    }
    else assignedZOrder = zOrder;

    while (canvases_.find(assignedZOrder) != canvases_.end())
    {
        ++assignedZOrder;
        if (zOrder == 0)
        {
            nextZOrder_ = assignedZOrder + 1;
        }
    }

    canvases_.emplace(assignedZOrder, canvas);

    canvas->GetPostEffectExecutor().RegisterCommandListToDirectX12(assignedZOrder);

    return assignedZOrder;
}

void OrderedCanvasLayer::RemoveCanvas(uint32_t zOrder)
{
    auto it = canvases_.find(zOrder);
    if (it != canvases_.end())
    {
        canvases_.erase(it);
    }
}

void OrderedCanvasLayer::RemoveCanvas(Canvas* canvas)
{
    for (auto it = canvases_.begin(); it != canvases_.end(); ++it)
    {
        if (it->second == canvas)
        {
            canvases_.erase(it);
            break;
        }
    }
}

void OrderedCanvasLayer::PreDraw()
{
    for (auto& canvas : canvases_)
    {
        canvas.second->GetPostEffectExecutor().PreDraw();
    }
}

void OrderedCanvasLayer::PostDraw()
{
    for (auto& canvas : canvases_)
    {
        canvas.second->GetPostEffectExecutor().PostDraw();
    }
}

void OrderedCanvasLayer::DrawObjects()
{
    // リストに格納された順番通りにCanvasのオブジェクトを描画する
    for (auto& canvas : canvases_)
    {
        if (IsCanvasEnabled(canvas.second))
        {
            canvas.second->DrawObjects();
        }
    }
}

void OrderedCanvasLayer::ApplyPostEffects()
{
    for (auto& canvas : canvases_)
    {
        if (IsCanvasEnabled(canvas.second))
        {
            canvas.second->ApplyPostEffects();
        }
    }
}

void OrderedCanvasLayer::DrawCanvases()
{
    for (auto& canvas : canvases_)
    {
        if (IsCanvasEnabled(canvas.second))
        {
            canvas.second->DrawCall(nullptr);
        }
    }
}
