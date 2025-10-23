#pragma once

#include <d3d12.h>
#include <string>
#include <list>
#include <unordered_map>

/// <summary>
/// ルートパラメータ管理クラス
/// </summary>
/// <typeparam name="_Size"> ルートパラメータの数 </typeparam>
template <int _Size>
class RootParameters
{
    using _me = RootParameters<_Size>;
    using _ref = _me&;

public:
    RootParameters() = default;
    ~RootParameters() = default;

    _ref SetParameter(
        int _index, 
        const std::string& _slot,
        D3D12_SHADER_VISIBILITY _visibility = D3D12_SHADER_VISIBILITY_ALL);

    const D3D12_ROOT_PARAMETER* GetParams() const { return params_; }
    int GetSize() const { return _Size; }
    uint32_t GetIndex(const std::string& _slot) const
    {
        auto it = slotMap_.find(_slot);
        return it != slotMap_.end() ? it->second : UINT32_MAX;
    }

private:
    std::unordered_map<std::string, uint32_t> slotMap_;
    D3D12_ROOT_PARAMETER params_[_Size] = {};
    std::list<D3D12_DESCRIPTOR_RANGE> ranges_;
};

#include "RootParameters.inl"