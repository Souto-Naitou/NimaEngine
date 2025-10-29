#include "DX12Resource.h"

#include <utility>
#include <config/EngineSetting.h>
#include "../SRVManager.h"

void DX12Resource::Initialize(
    Microsoft::WRL::ComPtr<ID3D12Resource> resource,
    D3D12_RESOURCE_STATES state,
    DXGI_FORMAT format,
    const std::string& name)
{
    this->resource_ = resource;
    stateTracker_.Initialize(this->resource_, state, format);
    if (this->resource_)
    {
        this->resource_->SetName(std::wstring(name.begin(), name.end()).c_str());
    }
}

void DX12Resource::CreateSRV()
{
    auto pSRVManager = SRVManager::GetInstance();

    /// 割り当て
    srvIndex_ = pSRVManager->Allocate();
    pSRVManager->CreateForTexture2D(
        srvIndex_,
        resource_.Get(),
        NimaEngine::Config::kRenderTargetFormat,
        1
    );

    /// ラッパークラスにSRV情報をセット
    srvHandleCPU_ = pSRVManager->GetCPUDescriptorHandle(srvIndex_);
    srvHandleGPU_ = pSRVManager->GetGPUDescriptorHandle(srvIndex_);
}

void DX12Resource::Reset()
{
    if (srvIndex_ != 0u) SRVManager::GetInstance()->Deallocate(srvIndex_);
    
    resource_.Reset();
    stateTracker_.Reset();
    srvIndex_ = 0u;
    srvHandleCPU_ = {};
    srvHandleGPU_ = {};
    rtvIndex_ = 0u;
    rtvHandleCPU_ = {};
}

void DX12Resource::SetRTV(uint32_t rtvIndex, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU)
{
    rtvIndex_ = rtvIndex;
    rtvHandleCPU_ = rtvHandleCPU;
}

void DX12Resource::SetSRV(uint32_t srvIndex, D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU, D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU)
{
    srvIndex_     = srvIndex;
    srvHandleCPU_ = srvHandleCPU;
    srvHandleGPU_ = srvHandleGPU;
}

uint32_t DX12Resource::GetSRVIndex() const
{
    return srvIndex_;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Resource::GetSRVHandleCPU() const
{
    return srvHandleCPU_;
}

D3D12_GPU_DESCRIPTOR_HANDLE DX12Resource::GetSRVHandleGPU() const
{
    return srvHandleGPU_;
}
