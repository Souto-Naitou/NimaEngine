#pragma once
#include <list>
#include <Features/Layer/Canvas.h>

class Layer
{
public:
    Layer() = default;
    ~Layer() = default;

    void DrawObjects();
    void ApplyPostEffects();
    void DrawCanvases();

private:
    std::list<Canvas*> canvases_;
};