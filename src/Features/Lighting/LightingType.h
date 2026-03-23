#pragma once
#include <Common/structs.h>

/// <summary>
/// ライティングの種類
/// </summary>
enum class LightingType : int32_t
{
    LambertianReflectance,
    HarfLambert
};

struct alignas(16) LightSetting
{
    int32_t         enableDirectionalLight;
    int32_t         enablePointLight;
    LightingType    lightingType;
    float           padding0;
};