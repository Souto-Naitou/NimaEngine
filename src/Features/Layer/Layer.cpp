#include "Layer.h"



bool IsCanvasEnabled(Canvas* canvas)
{
    bool isEnabled = canvas->IsEnabled();
    isEnabled &= !canvas->IsEnableManualDraw();
    return isEnabled;
}

void Layer::Initialize()
{

}

uint32_t Layer::AddCanvas(Canvas* canvas, uint32_t zOrder)
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

    canvas->GetPostEffectExecuter().RegisterCommandListToDirectX12(assignedZOrder);

    return assignedZOrder;
}

void Layer::RemoveCanvas(uint32_t zOrder)
{
    auto it = canvases_.find(zOrder);
    if (it != canvases_.end())
    {
        canvases_.erase(it);
    }
}

void Layer::RemoveCanvas(Canvas* canvas)
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

void Layer::PreDraw()
{
    for (auto& canvas : canvases_)
    {
        canvas.second->GetPostEffectExecuter().PreDraw();
    }
}

void Layer::PostDraw()
{
    for (auto& canvas : canvases_)
    {
        canvas.second->GetPostEffectExecuter().PostDraw();
    }
}

void Layer::DrawObjects()
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

void Layer::ApplyPostEffects()
{
    for (auto& canvas : canvases_)
    {
        if (IsCanvasEnabled(canvas.second))
        {
            canvas.second->ApplyPostEffects();
        }
    }
}

void Layer::DrawCanvases()
{
    for (auto& canvas : canvases_)
    {
        if (IsCanvasEnabled(canvas.second))
        {
            canvas.second->DrawCall(nullptr);
        }
    }
}
