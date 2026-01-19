#include "PostEffectInputCommon.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Window/Window.h>
#include <Features/DeltaTimeManager/DeltaTimeManager.h>


void PostEffectInputCommon::Initialize(ID3D12Device* pDevice)
{
    pDevice_ = pDevice;
    pTimeMeasurer_ = std::make_unique<TimeMeasurer>();
    pTimeMeasurer_->Start();
    this->CreateBufferResource();
}

void PostEffectInputCommon::Update()
{
    float dt = DeltaTimeManager::GetInstance()->GetDeltaTime(static_cast<uint32_t>(DeltaTimeChannelReserved::Default));
    pMappedData_->iDeltaTime = dt;
    ++(pMappedData_->iFrame);
    pMappedData_->iTime = pTimeMeasurer_->GetNow<float>();
}

void PostEffectInputCommon::CreateBufferResource()
{
    pBufferResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(RuntimeDataCommonGPU));
    pBufferResource_->Map(0, nullptr, reinterpret_cast<void**>(&pMappedData_));

    // 初期化
    pMappedData_->iResolution.x = static_cast<float>(Window::clientWidth);
    pMappedData_->iResolution.y = static_cast<float>(Window::clientHeight);
}
