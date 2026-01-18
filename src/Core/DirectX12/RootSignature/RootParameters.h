#pragma once

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
    inline const    D3D12_ROOT_PARAMETER*   GetParams() const { return params_.data(); }
    [[nodiscard]]
    inline          size_t                  GetSize() const { return params_.size(); }
    [[nodiscard]]
    inline          uint32_t                GetIndex(const std::string& slot) const
    {
        auto it = slotMap_.find(slot);
        return it != slotMap_.end() ? it->second : std::numeric_limits<uint32_t>::max();
    }

private:
    std::unordered_map<std::string, uint32_t> slotMap_;
    std::vector<D3D12_ROOT_PARAMETER> params_;
    std::vector<D3D12_DESCRIPTOR_RANGE> ranges_;
};