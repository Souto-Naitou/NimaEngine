#pragma once

#include <d3d12.h>
#include <wrl/client.h>

/// <summary>
/// パイプラインステートオブジェクトクラス
/// </summary>
class PipelineStateObject
{
    using _me = PipelineStateObject;
    using _ref = _me&;

public:
    PipelineStateObject() = default;
    ~PipelineStateObject() = default;

    _ref SetDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& _desc);

    _ref SetRootSignature(ID3D12RootSignature* _rootSignature_);
    _ref SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& _inputLayoutDesc);
    _ref SetVertexShader(const void* _pShaderByteCode, size_t _length);
    _ref SetPixelShader(const void* _pShaderByteCode, size_t _length);
    _ref SetRasterizerState(const D3D12_RASTERIZER_DESC& _rasterizerDesc);
    _ref SetBlendState(const D3D12_BLEND_DESC& _blendDesc);
    _ref SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& _depthStencilDesc);
    _ref SetSampleMask(UINT _sampleMask);
    _ref SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE _topologyType);
    _ref SetRenderTargetFormats(UINT _numRenderTargets, const DXGI_FORMAT* _rtvFormats, DXGI_FORMAT _dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT);
    _ref SetSampleDesc(const DXGI_SAMPLE_DESC& _sampleDesc);

    void Build(ID3D12Device* _device);


    ID3D12PipelineState* GetPSO() const { return pso_.Get(); }
    const D3D12_GRAPHICS_PIPELINE_STATE_DESC& GetDesc() const { return desc_; }

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_ = {};
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_ = nullptr;
};