#include "DirectionalLight.h"
#include <Core/DirectX12/Helper/DX12Helper.h>



void DirectionalLight::Initialize()
{
    /// リソースの作成
    /// CPUから読み取りは行わないため、readRangeのBeginとEndは0
    D3D12_RANGE readRange = { 0, 0 };
    pResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(value_type));
    pResource_->SetName(L"DirectionalLight::pResource_");
    pResource_->Map(0, &readRange, reinterpret_cast<void**>(&pMapped_));
}

void DirectionalLight::Update()
{
    data_.direction = data_.direction.Normalized();
    /// 書き込み
    *pMapped_ = data_;
}
