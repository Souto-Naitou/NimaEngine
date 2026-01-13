#include "DX12Resource.h"

#include <utility>
#include <config/EngineSetting.h>

DX12Resource::~DX12Resource()
{
    if (srvIndex_ != 0u) pSRVManager_->Deallocate(srvIndex_);
    if (rtvIndex_ != 0u) pRTVHeapCounter_->Deallocate(rtvIndex_);
}

void DX12Resource::Initialize(const Params& param)
{
    this->resource_ = param.resource;
    stateTracker_.Initialize(this->resource_, param.state, param.format);
    if (this->resource_)
    {
        this->resource_->SetName(std::wstring(param.name.begin(), param.name.end()).c_str());
    }

    pRTVHeapCounter_ = param.pRTVCounter;
    pSRVManager_ = SRVManager::GetInstance();
}

void DX12Resource::CreateSRV()
{
    /// 割り当て
    srvIndex_ = pSRVManager_->Allocate();
    pSRVManager_->CreateForTexture2D(
        srvIndex_,
        resource_.Get(),
        NimaEngine::Config::kRenderTargetFormat,
        1
    );

    /// ラッパークラスにSRV情報をセット
    srvHandleCPU_ = pSRVManager_->GetCPUDescriptorHandle(srvIndex_);
    srvHandleGPU_ = pSRVManager_->GetGPUDescriptorHandle(srvIndex_);
}

void DX12Resource::Reset()
{
    if (srvIndex_ != 0u) pSRVManager_->Deallocate(srvIndex_);
    if (rtvIndex_ != 0u) pRTVHeapCounter_->Deallocate(rtvIndex_);
    
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
