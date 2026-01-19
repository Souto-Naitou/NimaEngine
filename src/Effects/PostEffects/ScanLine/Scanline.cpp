#include "Scanline.h"
#include "Core/DirectX12/RootSignature/RootSignatureCache.h"
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Effects/PostEffects/GlobalInput/PostEffectInputCommon.h>
#include <imgui.h>
#include <config/EngineSetting.h>



void Scanline::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    pCommandList_ = desc.pCommandList;
    pDevice_ = pDx12_->GetDevice();

    // レンダーテクスチャ用リソースの生成
    pRenderTexture_ = std::make_unique<DX12Resource>();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, pDevice_, *pRenderTexture_, kName_ + "RenderTexture");

    // レンダーテクスチャのSRVを生成
    pRenderTexture_->CreateSRV();

    // ルートシグネチャの生成
    this->RegisterRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();
}

void Scanline::Finalize()
{
}

void Scanline::Setting()
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
    pCommandList_->SetGraphicsRootConstantBufferView(2, pOptionResource_->GetGPUVirtualAddress());
}

void Scanline::Apply()
{
    pCommandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void Scanline::ToShaderResourceState()
{
    pRenderTexture_->GetStateTracker().ChangeState(pCommandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Scanline::OnResizeBefore()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void Scanline::OnResizeAfter()
{
    throw std::logic_error("The method or operation is not implemented.");
}

void Scanline::DebugOverlay()
{
    #ifdef _DEBUG

    ImGui::DragFloat("Division", &pOption_->division, 0.01f);
    ImGui::DragFloat("Speed", &pOption_->speed, 0.01f);
    ImGui::ColorEdit4("Color0", &pOption_->color0.x);
    ImGui::ColorEdit4("Color1", &pOption_->color1.x);
    ImGui::SliderFloat("Opacity", &pOption_->opacity, 0.0f, 1.0f);
    bool isOverall = pOption_->isOverall > 0.5f;
    ImGui::Checkbox("Overall", &isOverall);
    pOption_->isOverall = isOverall ? 1.0f : 0.0f;
    
    #endif // _DEBUG
}

void Scanline::RegisterRootSignature()
{
    auto rsCache = RootSignatureCache::GetInstance();
    if (!rsCache->IsExist(kRootSignatureId_))
    {
        /// RootSignature作成
        RootSignatureDesc rootSignatureDesc = {};
        auto& rootParam = rootSignatureDesc.params;
        rootParam.SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam.SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam.SetParameter(2, "b1", D3D12_SHADER_VISIBILITY_PIXEL);
        auto& staticSampler = rootSignatureDesc.staticSamplers;
        staticSampler
            .PresetPointWrap()
            .SetMaxAnisotropy(16) // 最大異方性を16に設定
            .SetShaderRegister(0) // シェーダーレジスタ番号を0に設定
            .SetRegisterSpace(0); // レジスタスペースを0に設定

        /// ルートシグネチャ記述子を登録
        rsCache->Register(kRootSignatureId_, rootSignatureDesc);
    }

    pRootSignature_ = rsCache->GetOrCreate(kRootSignatureId_);
}

void Scanline::CreatePipelineStateObject()
{
    auto psoCache = PSOCache::GetInstance();

    /// PSOが登録されていないなら、生成するための情報を登録する
    if (!psoCache->IsExist(kPSOId_))
    {
        PSODesc desc{};
        desc.vs = kVertexShaderPath_;
        desc.ps = kPixelShaderPath_;
        desc.rootSignatureID = kRootSignatureId_;
        desc.blendState.Initialize(BlendDesc::BlendModes::Test);
        desc.primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        auto& rasterizerDesc = desc.rasterizerDesc;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.MultisampleEnable = TRUE;  // アンチエイリアス有効化
        rasterizerDesc.AntialiasedLineEnable = TRUE;  // ラインのアンチエイリアス有効化
        desc.inputLayoutDesc.pInputElementDescs = nullptr;
        desc.inputLayoutDesc.NumElements = 0;
        psoCache->Register(kPSOId_, desc);
    }

    /// 生成もしくは生成済みを取得する
    pPSO_ = psoCache->GetOrCreate(kPSOId_);
}

void Scanline::CreateResourceCBuffer()
{
    pOptionResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(ScanlineOption));
    pOptionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    // 初期化
    pOption_->division = 50.0f;
    pOption_->speed = 3.0f;
    pOption_->opacity = 1.0f;
    pOption_->color0 = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
    pOption_->color1 = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}