#pragma once

#include <d3d12.h>

/// <summary>
/// StaticSampler設定クラス
/// </summary>
class StaticSamplerDesc
{
private:
    using reference = StaticSamplerDesc&;

public:
    StaticSamplerDesc();
    ~StaticSamplerDesc() = default;

    // Setters
    reference SetFilter(D3D12_FILTER _filter);
    reference SetAddressU(D3D12_TEXTURE_ADDRESS_MODE _addressU);
    reference SetAddressV(D3D12_TEXTURE_ADDRESS_MODE _addressV);
    reference SetAddressW(D3D12_TEXTURE_ADDRESS_MODE _addressW);
    reference SetAddressUVW(D3D12_TEXTURE_ADDRESS_MODE _addressUVW);
    reference SetMipLODBias(float _mipLODBias);
    reference SetMaxAnisotropy(UINT _maxAnisotropy);
    reference SetComparisonFunc(D3D12_COMPARISON_FUNC _comparisonFunc);
    reference SetBorderColor(D3D12_STATIC_BORDER_COLOR _color);
    reference SetMinLOD(float _minLOD);
    reference SetMaxLOD(float _maxLOD);
    reference SetShaderRegister(UINT _shaderRegister);
    reference SetRegisterSpace(UINT _registerSpace);
    reference SetShaderVisibility(D3D12_SHADER_VISIBILITY _shaderVisibility);

    // Presets
    reference PresetPointWrap();
    reference PresetPointClamp();
    reference PresetLinearWrap();
    reference PresetLinearClamp();

    const D3D12_STATIC_SAMPLER_DESC& Get() const { return desc_; }

private:
    D3D12_STATIC_SAMPLER_DESC desc_ = {};
};