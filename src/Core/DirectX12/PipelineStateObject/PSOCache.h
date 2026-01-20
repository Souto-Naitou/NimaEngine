#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <unordered_map>
#include <string>
#include "PSODesc.h"

using PSOID = std::string;

class DirectX12;

class PSOCache
{
public:
    inline static PSOCache* GetInstance()
    {
        static PSOCache instance;
        return &instance;
    }

    inline void Initialize(DirectX12* pDx12) { pDx12_ = pDx12; }
    void Register(PSOID id, const PSODesc& desc);
    ID3D12PipelineState* GetOrCreate(const PSOID& id);
    bool IsExist(const PSOID& id) const
    {
        return psoCache_.find(id) != psoCache_.end();
    }

private:
    DirectX12* pDx12_ = nullptr;
    std::unordered_map<PSOID, PSODesc> descMap_;
    std::unordered_map<PSOID, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoCache_;
};