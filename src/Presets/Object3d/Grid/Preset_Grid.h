#pragma once
#include <memory>
#include <drawable/object3d/Object3d.h>
#include <Features/Model/IModel.h>

namespace presets::grid
{
    std::unique_ptr<Object3d> Create(IModel* pModel);
}