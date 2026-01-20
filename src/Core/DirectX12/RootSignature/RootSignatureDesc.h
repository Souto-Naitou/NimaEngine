#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include "RootParameters.h"
#include <Core/DirectX12/StaticSamplerDesc/StaticSamplerDesc.h>

/// <summary>
/// ルートシグネチャ記述子
/// </summary>
struct RootSignatureDesc
{
    RootParameters params;
    StaticSamplerDesc staticSamplers;
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    D3D12_ROOT_SIGNATURE_DESC ToD3D12RootSignatureDesc()
    {
        D3D12_ROOT_SIGNATURE_DESC d3dDesc{};
        d3dDesc.NumParameters = static_cast<UINT>(params.GetSize());
        d3dDesc.pParameters = params.BuildParams();
        d3dDesc.NumStaticSamplers = 1; // 一つだけ対応
        d3dDesc.pStaticSamplers = &staticSamplers.Get();
        d3dDesc.Flags = flags;
        return d3dDesc;
    }
};