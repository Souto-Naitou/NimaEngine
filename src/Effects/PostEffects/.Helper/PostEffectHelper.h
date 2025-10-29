#pragma once

#include <Core/DirectX12/DX12Resource/DX12Resource.h>
#include <d3d12.h>
#include <cstdint>

class DirectX12;

/// <summary>
/// ポストエフェクト用ヘルパー関数群
/// </summary>
namespace Helper
{
    void CreateRenderTexture(
        DirectX12*                      pDx12,
        ID3D12Device*                   pDevice,
        DX12Resource&                   resource,
        const std::string&              name
    );

    void CreateCommandList(
        ID3D12Device* pDevice,
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& Allocator
    );
}