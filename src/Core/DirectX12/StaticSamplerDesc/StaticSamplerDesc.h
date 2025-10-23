#pragma once

#include <d3d12.h>

/// <summary>
/// StaticSampler設定クラス
/// </summary>
class StaticSamplerDesc
{
private:
    using _me = StaticSamplerDesc;

public:
    StaticSamplerDesc();
    ~StaticSamplerDesc() = default;

    // Setters
    _me& SetFilter(D3D12_FILTER _filter);
    _me& SetAddressU(D3D12_TEXTURE_ADDRESS_MODE _addressU);
    _me& SetAddressV(D3D12_TEXTURE_ADDRESS_MODE _addressV);
    _me& SetAddressW(D3D12_TEXTURE_ADDRESS_MODE _addressW);
    _me& SetAddressUVW(D3D12_TEXTURE_ADDRESS_MODE _addressUVW);
    _me& SetMipLODBias(float _mipLODBias);
    _me& SetMaxAnisotropy(UINT _maxAnisotropy);
    _me& SetComparisonFunc(D3D12_COMPARISON_FUNC _comparisonFunc);
    _me& SetBorderColor(D3D12_STATIC_BORDER_COLOR _color);
    _me& SetMinLOD(float _minLOD);
    _me& SetMaxLOD(float _maxLOD);
    _me& SetShaderRegister(UINT _shaderRegister);
    _me& SetRegisterSpace(UINT _registerSpace);
    _me& SetShaderVisibility(D3D12_SHADER_VISIBILITY _shaderVisibility);

    // Presets
    _me& PresetPointWrap();
    _me& PresetPointClamp();
    _me& PresetLinearWrap();
    _me& PresetLinearClamp();

    const D3D12_STATIC_SAMPLER_DESC& Get() const { return desc_; }

private:
    D3D12_STATIC_SAMPLER_DESC desc_ = {};
};