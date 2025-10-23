#include "Preset_Grid.h"
#include <DebugTools/Logger/Logger.h>

std::unique_ptr<Object3d> presets::grid::Create(IModel* pModel)
{
    if (!pModel)
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "Model pointer is null.");
        return std::unique_ptr<Object3d>();
    }

    auto pGrid = std::make_unique<Object3d>();
    pGrid->Initialize();
    pGrid->SetName("Grid");
    pGrid->SetModel(pModel);

    auto& option = pGrid->GetOption();
    option.transform.scale = { 1.0f, 1.0f, 1.0f };
    option.tilingData->tilingMultiply = { 100.0f, 100.0f };
    option.lightingData->enableLighting = false;
    option.materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    option.materialData->environmentCoefficient = 0.0f;

    return pGrid;
}
