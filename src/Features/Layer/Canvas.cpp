#include "Canvas.h"



void Canvas::Initialize(const std::string& name)
{
    pDebugEntry_ = std::make_unique<DebugEntry<Canvas>>("Canvas", name, this);
}

void Canvas::ImGui()
{


}

Canvas& Canvas::RegisterDrawable(Sprite* sprite)
{
    sprites_.push_back(sprite);
    return *this;
}