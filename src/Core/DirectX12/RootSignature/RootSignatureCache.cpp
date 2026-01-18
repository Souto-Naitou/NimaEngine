#include "RootSignatureCache.h"
#include <DebugTools/Logger/Logger.h>


void RootSignatureCache::Register(const RootSignatureID& id, const RootSignatureDesc& desc)
{
    descMap_[id] = desc;
}

ID3D12RootSignature* RootSignatureCache::GetOrCreate(RootSignatureID id)
{
    auto it = descMap_.find(id);
    if (it == descMap_.end())
    {
        return nullptr;
    }

    const RootSignatureDesc& desc = it->second;

    if (rootSignatureMap_.find(id) != rootSignatureMap_.end())
    {
        return rootSignatureMap_[id].Get();
    }

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    auto d3d12RootSigDesc = desc.ToD3D12RootSignatureDesc();
    HRESULT hr = D3D12SerializeRootSignature(
        &d3d12RootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob
    );

    if (FAILED(hr))
    {
        // エラーハンドリング（ログ出力など）
        LOG_FATAL(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        #ifdef _DEBUG
        assert(false);
        #else
        std::terminate();
        #endif // _DEBUG

        return nullptr;
    }

    auto& rootSignature = rootSignatureMap_[id];

    // デバイスの取得方法は環境によって異なるため、適切に取得してください
    hr = pDevice_->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature));

    if (FAILED(hr))
    {
        LOG_FATAL("CreateRootSignature Failed");
        return nullptr;
    }

    return rootSignature.Get();
}
