#pragma once

#include <d3d12.h>
#include <string>
#include <Core/DirectX12/BlendDesc.h>
#include <Core/DirectX12/RootSignature/RootSignatureCache.h>

struct PSODesc
{
    using ShaderPath = std::wstring;

    ShaderPath vs, ps;
    BlendDesc blendState;
    RootSignatureID rootSignatureID;
    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};