#include "Layer.h"



uint32_t Layer::AddCanvas(Canvas* canvas, uint32_t zOrder)
{
    if (zOrder == 0)
    {
        canvases_.emplace(nextZOrder_, canvas);
        return nextZOrder_++;
    }

    while (canvases_.find(zOrder) != canvases_.end())
    {
        ++zOrder;
    }

    canvases_.emplace(zOrder, canvas);

    return zOrder;
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
        if (canvas.second->IsEnabled())
        {
            canvas.second->DrawObjects();
        }
    }
}

void Layer::ApplyPostEffects()
{
    for (auto& canvas : canvases_)
    {
        if (canvas.second->IsEnabled())
        {
            canvas.second->ApplyPostEffects();
        }
    }
}

void Layer::DrawCanvases()
{
    for (auto& canvas : canvases_)
    {
        if (canvas.second->IsEnabled())
        {
            canvas.second->Draw();
        }
    }
}
