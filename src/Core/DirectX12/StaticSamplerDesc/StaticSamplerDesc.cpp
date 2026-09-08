#include "./StaticSamplerDesc.h"

using reference = StaticSamplerDesc&;

StaticSamplerDesc::StaticSamplerDesc()
{
    desc_.MipLODBias = 0.0f;                                    // ミップマップのオフセット
    desc_.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;         // 比較なし
    desc_.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE; // ボーダーカラー
    desc_.MinLOD = 0.0f;                                        // 最小ミップレベル
    desc_.MaxLOD = D3D12_FLOAT32_MAX;                           // 最大ミップレベル
    desc_.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;     // ピクセルシェーダーで使用
}

#pragma region "Setters"

    reference StaticSamplerDesc::SetFilter(D3D12_FILTER _filter)
    {
        desc_.Filter = _filter;
        return *this;
    }

    reference StaticSamplerDesc::SetAddressU(D3D12_TEXTURE_ADDRESS_MODE _addressU)
    {
        desc_.AddressU = _addressU;
        return *this;
    }

    reference StaticSamplerDesc::SetAddressV(D3D12_TEXTURE_ADDRESS_MODE _addressV)
    {
        desc_.AddressV = _addressV;
        return *this;
    }

    reference StaticSamplerDesc::SetAddressW(D3D12_TEXTURE_ADDRESS_MODE _addressW)
    {
        desc_.AddressW = _addressW;
        return *this;
    }

    reference StaticSamplerDesc::SetAddressUVW(D3D12_TEXTURE_ADDRESS_MODE _addressUVW)
    {
        desc_.AddressU = _addressUVW;
        desc_.AddressV = _addressUVW;
        desc_.AddressW = _addressUVW;
        return *this;
    }

    reference StaticSamplerDesc::SetMipLODBias(float _mipLODBias)
    {
        desc_.MipLODBias = _mipLODBias;
        return *this;
    }

    reference StaticSamplerDesc::SetMaxAnisotropy(UINT _maxAnisotropy)
    {
        desc_.MaxAnisotropy = _maxAnisotropy;
        return *this;
    }

    reference StaticSamplerDesc::SetComparisonFunc(D3D12_COMPARISON_FUNC _comparisonFunc)
    {
        desc_.ComparisonFunc = _comparisonFunc;
        return *this;
    }

    reference StaticSamplerDesc::SetBorderColor(D3D12_STATIC_BORDER_COLOR _color)
    {
        desc_.BorderColor = _color;
        return *this;
    }

    reference StaticSamplerDesc::SetMinLOD(float _minLOD)
    {
        desc_.MinLOD = _minLOD;
        return *this;
    }

    reference StaticSamplerDesc::SetMaxLOD(float _maxLOD)
    {
        desc_.MaxLOD = _maxLOD;
        return *this;
    }

    reference StaticSamplerDesc::SetShaderRegister(UINT _shaderRegister)
    {
        desc_.ShaderRegister = _shaderRegister;
        return *this;
    }

    reference StaticSamplerDesc::SetRegisterSpace(UINT _registerSpace)
    {
        desc_.RegisterSpace = _registerSpace;
        return *this;
    }

    reference StaticSamplerDesc::SetShaderVisibility(D3D12_SHADER_VISIBILITY _shaderVisibility)
    {
        desc_.ShaderVisibility = _shaderVisibility;
        return *this;
    }

#pragma endregion // "Setters"

#pragma region "Presets"

    reference StaticSamplerDesc::PresetLinearClamp()
    {
        return SetFilter(D3D12_FILTER_MIN_MAG_MIP_LINEAR)
            .SetAddressUVW(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    }

    reference StaticSamplerDesc::PresetLinearWrap()
    {
        return SetFilter(D3D12_FILTER_MIN_MAG_MIP_LINEAR)
            .SetAddressUVW(D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    }

    reference StaticSamplerDesc::PresetPointClamp()
    {
        return SetFilter(D3D12_FILTER_MIN_MAG_MIP_POINT)
            .SetAddressUVW(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    }

    reference StaticSamplerDesc::PresetPointWrap()
    {
        return SetFilter(D3D12_FILTER_MIN_MAG_MIP_POINT)
            .SetAddressUVW(D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    }

#pragma endregion // "Presets"