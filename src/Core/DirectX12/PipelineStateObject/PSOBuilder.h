#pragma once

#include <d3d12.h>
#include <wrl/client.h>

/// <summary>
/// パイプラインステートオブジェクトクラス
/// </summary>
class PSOBuilder
{
    using _me = PSOBuilder;
    using _ref = _me&;

public:
    PSOBuilder() = default;
    ~PSOBuilder() = default;

    _ref SetDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);

    _ref SetRootSignature(ID3D12RootSignature* rootSignature_);
    _ref SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& inputLayoutDesc);
    _ref SetVertexShader(const void* pShaderByteCode, size_t length);
    _ref SetPixelShader(const void* pShaderByteCode, size_t length);
    _ref SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc);
    _ref SetBlendState(const D3D12_BLEND_DESC& blendDesc);
    _ref SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);
    _ref SetSampleMask(UINT sampleMask);
    _ref SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);
    _ref SetRenderTargetFormats(UINT numRenderTargets, const DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
    _ref SetSampleDesc(const DXGI_SAMPLE_DESC& sampleDesc);

    Microsoft::WRL::ComPtr<ID3D12PipelineState> Build(ID3D12Device* device);


    const D3D12_GRAPHICS_PIPELINE_STATE_DESC& GetDesc() const { return desc_; }

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_ = {};
};