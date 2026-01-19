#include "PSOCache.h"
#include "PSOBuilder.h"
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Utility/ConvertString/ConvertString.h>
#include <config/EngineSetting.h>

void PSOCache::Register(PSOID key, const PSODesc& desc)
{
    descMap_[key] = desc;
}

ID3D12PipelineState* PSOCache::GetOrCreate(const PSOID& id)
{
    /// DESCを検索 (登録されていなければ終了)
    auto it = descMap_.find(id);
    if (it == descMap_.end())
    {
        return nullptr;
    }

    /// PSOキャッシュを検索 (存在すればそれを返す)
    const PSODesc& desc = it->second;
    if (psoCache_.find(id) != psoCache_.end())
    {
        return psoCache_[id].Get();
    }

    /// ShaderBlobを生成
    Microsoft::WRL::ComPtr<IDxcBlob> vsBlob, psBlob;

    /// 以下、PSOを新規作成する処理
    /// シェーダーのコンパイル
    IDxcUtils* dxcUtils = pDx12_->GetDxcUtils();
    IDxcCompiler3* dxcCompiler = pDx12_->GetDxcCompiler();
    IDxcIncludeHandler* includeHandler = pDx12_->GetIncludeHandler();
    vsBlob = DX12Helper::CompileShader(desc.vs, L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vsBlob != nullptr);
    psBlob = DX12Helper::CompileShader(desc.ps, L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(psBlob != nullptr);

    /// PSOを生成してキャッシュに登録
    PSOBuilder builder;
    psoCache_[id] = builder
        .SetRootSignature(RootSignatureCache::GetInstance()->GetOrCreate(desc.rootSignatureID))
        .SetInputLayout(desc.inputLayoutDesc)
        .SetVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize())
        .SetPixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize())
        .SetRasterizerState(desc.rasterizerDesc)
        .SetBlendState(desc.blendState.Get())
        .SetPrimitiveTopologyType(desc.primitiveTopologyType)
        .SetRenderTargetFormats(1, &NimaEngine::Config::kRenderTargetFormat, DXGI_FORMAT_D24_UNORM_S8_UINT)
        .SetSampleDesc({1, 0})
        .SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK)
        .Build(pDx12_->GetDevice());
    return psoCache_[id].Get();
}
