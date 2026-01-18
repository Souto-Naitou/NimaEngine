#pragma once
#include <d3d12.h>
#include <cstdint>
#include <unordered_map>
#include "RootSignatureDesc.h"

using RootSignatureID = std::string;

class RootSignatureCache
{
public:
    
    inline static RootSignatureCache* GetInstance()
    {
        static RootSignatureCache instance;
        return &instance;
    }

    /// <summary>
    /// 初期化します。
    /// </summary>
    /// <param name="device">D3D12 デバイス。</param>
    inline void Initialize(ID3D12Device* device) { pDevice_ = device; }

    /// <summary>
    /// ルートシグネチャを登録します。
    /// </summary>
    /// <param name="id">登録するルートシグネチャのID。</param>
    /// <param name="desc">登録するルートシグネチャの記述子。</param>
    void Register(const RootSignatureID& id, const RootSignatureDesc& desc);

    /// <summary>
    /// 指定されたIDに基づいてルートシグネチャを取得または作成します。
    /// </summary>
    /// <param name="id">取得または作成するルートシグネチャのID。</param>
    /// <returns>ルートシグネチャへのポインタ。</returns>
    ID3D12RootSignature* GetOrCreate(RootSignatureID id);

    inline bool IsExist(const RootSignatureID& id) const
    {
        return rootSignatureMap_.find(id) != rootSignatureMap_.end();
    }

private:
    ID3D12Device* pDevice_ = nullptr;
    std::unordered_map<RootSignatureID, RootSignatureDesc> descMap_;
    std::unordered_map<RootSignatureID, Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatureMap_;
};