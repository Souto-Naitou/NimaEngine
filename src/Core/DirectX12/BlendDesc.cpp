#include "BlendDesc.h"

#include <cassert>

void BlendDesc::Initialize(BlendModes _mode)
{
    desc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    desc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    desc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    desc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    desc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    ChangeMode(_mode);
}

void BlendDesc::ChangeMode(BlendModes _mode)
{
    currentMode_ = _mode;
    switch (_mode)
    {
    case BlendDesc::BlendModes::None:
        desc_.RenderTarget[0].BlendEnable = false;
        break;

    case BlendDesc::BlendModes::Normal:
    case BlendDesc::BlendModes::Alpha:
        desc_.RenderTarget[0].BlendEnable = true;
        desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        desc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        break;

    case BlendDesc::BlendModes::Add:
        desc_.RenderTarget[0].BlendEnable = true;
        desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        desc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;

    case BlendDesc::BlendModes::Subtract:
        desc_.RenderTarget[0].BlendEnable = true;
        desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
        desc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;

    case BlendDesc::BlendModes::Multiply:
        desc_.RenderTarget[0].BlendEnable = true;
        desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
        desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        desc_.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
        break;

    case BlendDesc::BlendModes::Screen:
        desc_.RenderTarget[0].BlendEnable = true;
        desc_.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
        desc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        desc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        break;

    case BlendDesc::BlendModes::COUNT:
        assert(0 && "BlendModeにCOUNTが指定されています。");
        break;
    }
}
