#pragma once
#undef max
#include <d3d12.h>
#include <string>
#include <list>
#include <unordered_map>
#include <vector>
#include <limits>

/// <summary>
/// ルートパラメータ管理クラス
/// </summary>
class RootParameters
{
    using _me = RootParameters;
    using _ref = _me&;

public:
    RootParameters() = default;
    ~RootParameters() = default;

    _ref SetParameter(
        int index, 
        const std::string& slot,
        D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

    [[nodiscard]]
    const           D3D12_ROOT_PARAMETER*   BuildParams();
    [[nodiscard]]
    inline          size_t                  GetSize() const { return params_.size(); }

private:
    struct DescriptorTableInfo
    {
        uint32_t rangeOffset;
        uint32_t rangeCount;
    };

    struct RootParameterInfo
    {
        D3D12_ROOT_PARAMETER_TYPE type;
        union
        {
            DescriptorTableInfo table;
            D3D12_ROOT_DESCRIPTOR descriptor;
            D3D12_ROOT_CONSTANTS constants;
        };
        D3D12_SHADER_VISIBILITY visibility;
    };

    std::unordered_map<std::string, std::vector<uint32_t>> slotMap_;
    std::vector<RootParameterInfo> params_;
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges_;
    std::vector<D3D12_ROOT_PARAMETER> d3dParams_;
};