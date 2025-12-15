#include "SRVManager.h"
#include <Core/DirectX12/Helper/DX12HeapHelper.h>

#ifdef _DEBUG
#include <DebugTools/DebugManager/DebugManager.h>
#include <DebugTools/ImGuiTemplates/ImGuiTemplates.h>
#endif // _DEBUG

void SRVManager::Initialize(DirectX12* pDx12)
{
#ifdef _DEBUG
    DebugManager::GetInstance()->SetComponent("Core", name_, std::bind(&SRVManager::ImGui, this), true);
#endif // _DEBUG

    name_ = "SRVManager";

    pDx12_ = pDx12;
    pDescHeap_ = DX12HeapHelper::CreateDescriptorHeap(
        pDx12_->GetDevice(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        kMaxSRVCount_,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
    );
    descriptorSize_ = pDx12_->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void SRVManager::SetDescriptorHeaps()
{
    ID3D12DescriptorHeap* descriptorHeaps[] = { pDescHeap_.Get() };
    pDx12_->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
}

uint32_t SRVManager::Allocate()
{
    assert(currentIndex_ < kMaxSRVCount_ && "インデックスが最大値を超えています");

    currentSize_++;

    for (uint32_t i = 0; i < kMaxSRVCount_; ++i)
    {
        uint32_t idx = (currentIndex_ + i) % kMaxSRVCount_;
        if (!isAllocated_[idx])
        {
            isAllocated_[idx] = true;
            currentIndex_ = (idx + 1) % kMaxSRVCount_;
            return idx;
        }
    }

    assert(false && "SRVの割り当てに失敗しました");

    return 0xffffffff;
}

void SRVManager::Deallocate(uint32_t index)
{
    assert(isAllocated_[index] && "すでに開放されています。");
    isAllocated_[index] = false;
    currentSize_--;

    return;
}

D3D12_CPU_DESCRIPTOR_HANDLE SRVManager::GetCPUDescriptorHandle(uint32_t index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = pDescHeap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += (descriptorSize_ * index);
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE SRVManager::GetGPUDescriptorHandle(uint32_t index)
{
    D3D12_GPU_DESCRIPTOR_HANDLE handle = pDescHeap_->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += (descriptorSize_ * index);
    return handle;
}

void SRVManager::SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t srvIndex)
{
    pDx12_->GetCommandList()->SetGraphicsRootDescriptorTable(rootParameterIndex, GetGPUDescriptorHandle(srvIndex));
}

void SRVManager::CreateForTexture2D(uint32_t index, ID3D12Resource* pTexture, DXGI_FORMAT format, UINT mipLevels)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                  = format;
    srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels     = mipLevels;

    pDx12_->GetDevice()->CreateShaderResourceView(pTexture, &srvDesc, GetCPUDescriptorHandle(index));
}

void SRVManager::CreateForCubemap(uint32_t index, ID3D12Resource* pTexture, DXGI_FORMAT format, UINT mipLevels)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                      = format;
    srvDesc.Shader4ComponentMapping     = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension               = D3D12_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels       = mipLevels;
    srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

    pDx12_->GetDevice()->CreateShaderResourceView(pTexture, &srvDesc, GetCPUDescriptorHandle(index));
}

void SRVManager::CreateForStructuredBuffer(uint32_t index, ID3D12Resource* pBuffer, UINT numElements, UINT stride)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                     = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement        = 0;
    srvDesc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
    srvDesc.Buffer.NumElements         = numElements;
    srvDesc.Buffer.StructureByteStride = stride;

    pDx12_->GetDevice()->CreateShaderResourceView(pBuffer, &srvDesc, GetCPUDescriptorHandle(index));
}

void SRVManager::CreateUAV(uint32_t index, ID3D12Resource* pTexture, DXGI_FORMAT format)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Format             = format;
    uavDesc.Texture2D.MipSlice = 0;

    pDx12_->GetDevice()->CreateUnorderedAccessView(pTexture, nullptr, &uavDesc, GetCPUDescriptorHandle(index));
}

void SRVManager::CreateUAV4Buffer(uint32_t index, ID3D12Resource* pTexture, DXGI_FORMAT format, uint32_t numElements, uint32_t structureByteStride)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format             = format;
    uavDesc.ViewDimension      = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements  = numElements;
    uavDesc.Buffer.Flags        = D3D12_BUFFER_UAV_FLAG_NONE;
    uavDesc.Buffer.StructureByteStride = structureByteStride;

    pDx12_->GetDevice()->CreateUnorderedAccessView(pTexture, nullptr, &uavDesc, GetCPUDescriptorHandle(index));
}

void SRVManager::ImGui()
{
#ifdef _DEBUG

    auto pFunc = [&]()
    {
        ImGuiTemplate::VariableTableRow("SRV許容数", kMaxSRVCount_);
        ImGuiTemplate::VariableTableRow("現在のSRV数", currentSize_);
    };
    ImGuiTemplate::VariableTable("SRVManager", pFunc);

#endif // _DEBUG
}