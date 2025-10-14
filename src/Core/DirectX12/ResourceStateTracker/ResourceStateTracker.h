#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <string>
#include <utility>


class ResourceStateTracker
{
public:
    void Initialize(
        Microsoft::WRL::ComPtr<ID3D12Resource> pResource,
        D3D12_RESOURCE_STATES state,
        DXGI_FORMAT format
    );

    D3D12_RESOURCE_STATES GetState() const { return state_; }
    const DXGI_FORMAT& GetFormat() const { return format_; }

    void Reset();
    void ChangeState(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState);

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> pResource_;
    D3D12_RESOURCE_STATES state_;
    DXGI_FORMAT format_;
};