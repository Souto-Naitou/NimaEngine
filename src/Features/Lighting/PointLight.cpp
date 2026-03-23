#include "PointLight.h"
#include <drawable/object3d/Object3d.h>

#include <sstream>
#include <cstdarg>
#include <Core/DirectX12/Helper/DX12Helper.h>

void PointLight::Initialize()
{
    std::stringstream ss;
    ss << "PointLight##0x" << std::hex << this;

    this->CreateGPUResource();
}

void PointLight::Update()
{
    // GPUにデータを転送
    *pMapped_ = data_;
}

void PointLight::CreateGPUResource()
{
    pResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(value_type));
    /// CPUから読み取りは行わないため、readRangeのBeginとEndは0
    D3D12_RANGE readRange = { 0, 0 };
    pResource_->Map(0, &readRange, reinterpret_cast<void**>(&pMapped_));
}
