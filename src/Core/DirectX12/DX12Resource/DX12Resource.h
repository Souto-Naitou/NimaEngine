#pragma once

#include <wrl/client.h>
#include <d3d12.h>
#include <cstdint>
#include <string>
#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>
#include <dxgiformat.h>

/// <summary>
/// ID3D12Resourceと各種DescriptorHandleをまとめたラッパークラス
/// </summary>
class DX12Resource
{
public:
    DX12Resource() = default;
    ~DX12Resource() = default;

    void Initialize(
        Microsoft::WRL::ComPtr<ID3D12Resource> resource,
        D3D12_RESOURCE_STATES state,
        DXGI_FORMAT format,
        const std::string& name = "Unnamed(Managed by DX12Resource)"
    );

    void Reset();

    // Setter
    void SetRTV(uint32_t rtvIndex, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU);
    void SetSRV(uint32_t srvIndex, D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU, D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU);

    // Getter
    [[nodiscard]] const ResourceStateTracker&           GetStateTracker() const { return stateTracker_; }
    [[nodiscard]] uint32_t                              GetSRVIndex() const;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE           GetSRVHandleCPU() const;
    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE           GetSRVHandleGPU() const;
    [[nodiscard]] const D3D12_CPU_DESCRIPTOR_HANDLE&    GetRTVHandle() const { return rtvHandleCPU_; }
    [[nodiscard]] uint32_t                              GetRTVIndex() const { return rtvIndex_; }

    // Getter (Ref)
    [[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Resource>& GetResource() { return resource_; }
    [[nodiscard]] const Microsoft::WRL::ComPtr<ID3D12Resource>& GetResource() const { return resource_; }
    [[nodiscard]] ResourceStateTracker& GetStateTracker() { return stateTracker_; }

private:
    // Resource
    Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;

    // Stateの整合性補助クラス
    ResourceStateTracker                    stateTracker_ = {};

    // SRV
    uint32_t                                srvIndex_       = 0u;
    D3D12_CPU_DESCRIPTOR_HANDLE             srvHandleCPU_   = {};
    D3D12_GPU_DESCRIPTOR_HANDLE             srvHandleGPU_   = {};

    // RTV
    uint32_t                                rtvIndex_       = 0u;
    D3D12_CPU_DESCRIPTOR_HANDLE             rtvHandleCPU_   = {};
};