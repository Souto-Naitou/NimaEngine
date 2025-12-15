#include "./PipelineStateObject.h"
#include <stdexcept>

using _me = PipelineStateObject;
using _ref = _me&;

_ref PipelineStateObject::SetDesc(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc)
{
    desc_ = desc;
    return *this;
}

_ref PipelineStateObject::SetRootSignature(ID3D12RootSignature* pRootSignature)
{
    desc_.pRootSignature = pRootSignature;
    return *this;
}

_ref PipelineStateObject::SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& _inputLayoutDesc)
{
    desc_.InputLayout = _inputLayoutDesc;
    return *this;
}

_ref PipelineStateObject::SetVertexShader(const void* _pShaderByteCode, size_t _length)
{
    desc_.VS.pShaderBytecode = _pShaderByteCode;
    desc_.VS.BytecodeLength = _length;
    return *this;
}

_ref PipelineStateObject::SetPixelShader(const void* pShaderByteCode, size_t length)
{
    desc_.PS.pShaderBytecode = pShaderByteCode;
    desc_.PS.BytecodeLength = length;
    return *this;
}

_ref PipelineStateObject::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc)
{
    desc_.RasterizerState = rasterizerDesc;
    return *this;
}

_ref PipelineStateObject::SetBlendState(const D3D12_BLEND_DESC& blendDesc)
{
    desc_.BlendState = blendDesc;
    return *this;
}

_ref PipelineStateObject::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc)
{
    desc_.DepthStencilState = depthStencilDesc;
    return *this;
}

_ref PipelineStateObject::SetSampleMask(UINT sampleMask)
{
    desc_.SampleMask = sampleMask;
    return *this;
}

_ref PipelineStateObject::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE _topologyType)
{
    desc_.PrimitiveTopologyType = _topologyType;
    return *this;
}

_ref PipelineStateObject::SetRenderTargetFormats(UINT _numRenderTargets, const DXGI_FORMAT* _rtvFormats, DXGI_FORMAT _dsvFormat)
{
    desc_.NumRenderTargets = _numRenderTargets;
    for (UINT i = 0; i < _numRenderTargets; ++i)
    {
        desc_.RTVFormats[i] = _rtvFormats[i];
    }
    desc_.DSVFormat = _dsvFormat;
    return *this;
}

_ref PipelineStateObject::SetSampleDesc(const DXGI_SAMPLE_DESC& _sampleDesc)
{
    desc_.SampleDesc = _sampleDesc;
    return *this;
}

void PipelineStateObject::Build(ID3D12Device* _device)
{
    HRESULT hr = _device->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(&pso_));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create graphics pipeline state.");
    }
}
