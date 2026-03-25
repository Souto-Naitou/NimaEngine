#pragma once
#include <Features/Lighting/DirectionalLight.h>
#include <Features/Lighting/PointLight.h>

/// ライト集合
struct LightRepository
{
    DirectionalLight*   pDirectionalLight_  = nullptr;
    PointLight*         pPointLights_       = {};
};