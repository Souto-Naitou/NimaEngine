#include "PostEffectBase.h"
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include "Effects/PostEffects/GlobalInput/PostEffectInputCommon.h"
#include "config/EngineSetting.h"


void PostEffectBase::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    pCommandList_ = desc.pCommandList;

    this->CheckInvariants();

    pDevice_ = pDx12_->GetDevice();

    // レンダーテクスチャ用リソースの生成
    pRenderTexture_ = std::make_unique<DX12Resource>();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, pDevice_, *pRenderTexture_, name_ + "RenderTexture");

    // レンダーテクスチャのSRVを生成
    pRenderTexture_->CreateSRV();

    // ルートシグネチャの生成
    this->RegisterRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 定数バッファの生成
    this->CreateCBuffer(pDevice_);
}

void PostEffectBase::Finalize()
{
}

void PostEffectBase::Setting()
{
    // レンダーテクスチャをレンダーターゲット状態に変更
    pRenderTexture_->GetStateTracker().ChangeState(pCommandList_, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // クリア
    pCommandList_->ClearRenderTargetView(pRenderTexture_->GetRTVHandle(), &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    // レンダーターゲットを設定 (自分が所有するテクスチャに対して設定)
    pCommandList_->OMSetRenderTargets(1, &pRenderTexture_->GetRTVHandle(), FALSE, nullptr);

    // PSOとルートシグネチャを設定
    pCommandList_->SetGraphicsRootSignature(pRootSignature_);
    pCommandList_->SetPipelineState(pPSO_);

    // 入力テクスチャのSRVを設定する（自分が所有するテクスチャのSRVではないため注意)
    pCommandList_->SetGraphicsRootDescriptorTable(0, inputGpuHandle_);

    pCommandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    auto commonInput = PostEffectInputCommon::GetInstance()->GetBufferResource();
    pCommandList_->SetGraphicsRootConstantBufferView(1, commonInput->GetGPUVirtualAddress());

    /// 追加で定数バッファを使用する場合は、CreateCBufferとSetCBufferをオーバーライドして対応してください。
    this->SetCBuffer(pCommandList_);
}

void PostEffectBase::Apply()
{
    pCommandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void PostEffectBase::ToShaderResourceState()
{
    pRenderTexture_->GetStateTracker().ChangeState(pCommandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void PostEffectBase::RegisterRootSignature()
{
    auto rsCache = RootSignatureCache::GetInstance();
    if (!rsCache->IsExist(rootSignatureId_))
    {
        /// RootSignature作成
        RootSignatureDesc rootSignatureDesc = {};
        rootSignatureDesc.params
            .SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL)
            .SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_PIXEL);
        this->RegisterAdditionalRootParameter(rootSignatureDesc.params);

        /// サンプラーの設定
        rootSignatureDesc.staticSamplers
            .PresetPointWrap()
            .SetMaxAnisotropy(16) // 最大異方性を16に設定
            .SetShaderRegister(0) // シェーダーレジスタ番号を0に設定
            .SetRegisterSpace(0); // レジスタスペースを0に設定

        /// ルートシグネチャ記述子を登録
        rsCache->Register(rootSignatureId_, rootSignatureDesc);
    }

    pRootSignature_ = rsCache->GetOrCreate(rootSignatureId_);
}

void PostEffectBase::CreatePipelineStateObject()
{
    auto psoCache = PSOCache::GetInstance();

    /// PSOが登録されていないなら、生成するための情報を登録する
    if (!psoCache->IsExist(psoId_))
    {
        PSODesc desc{};
        desc.vs = vertexShaderPath_;
        desc.ps = pixelShaderPath_;
        desc.rootSignatureID = rootSignatureId_;
        desc.blendState.Initialize(BlendDesc::BlendModes::Test);
        desc.primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        auto& rasterizerDesc = desc.rasterizerDesc;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.MultisampleEnable = TRUE;  // アンチエイリアス有効化
        rasterizerDesc.AntialiasedLineEnable = TRUE;  // ラインのアンチエイリアス有効化
        desc.inputLayoutDesc.pInputElementDescs = nullptr;
        desc.inputLayoutDesc.NumElements = 0;
        psoCache->Register(psoId_, desc);
    }

    /// 生成もしくは生成済みを取得する
    pPSO_ = psoCache->GetOrCreate(psoId_);
}

void PostEffectBase::CheckInvariants()
{
    assert(!psoId_.empty() && "PSO ID must be set.");
    assert(!rootSignatureId_.empty() && "Root Signature ID must be set.");
    assert(!pixelShaderPath_.empty() && "Pixel Shader Path must be set.");
    assert(pDx12_ != nullptr && "DirectX12 pointer must be set.");
    assert(pCommandList_ != nullptr && "D3D12 Graphics Command List pointer must be set.");
}
