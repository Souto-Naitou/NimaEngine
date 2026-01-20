#include "BoxFilter.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <imgui.h>

void BoxFilter::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;
    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "BoxFilterRenderTexture");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();

    // ルートシグネチャの生成
    this->RegisterRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();
}

void BoxFilter::Enable(bool flag)
{
    isEnabled_ = flag;
}

void BoxFilter::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    inputGpuHandle_ = gpuHandle;
}

bool BoxFilter::Enabled() const
{
    return isEnabled_;
}

D3D12_GPU_DESCRIPTOR_HANDLE BoxFilter::GetOutputTextureHandle() const
{
    return renderTexture_.GetSRVHandleGPU();
}

const std::string& BoxFilter::GetName() const
{
    return name_;
}

DX12Resource* BoxFilter::GetOutputResource() const
{
    return const_cast<DX12Resource*>(&renderTexture_);
}

BoxFilterOption& BoxFilter::GetOption()
{
    return *pOption_;
}

const BoxFilterOption& BoxFilter::GetOption() const
{
    return *pOption_;
}

void BoxFilter::Apply()
{
    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void BoxFilter::Finalize()
{
}

void BoxFilter::Setting()
{
    // レンダーテクスチャをレンダーターゲット状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // レンダーターゲットを設定 (自分が所有するテクスチャに対して設定)
    commandList_->OMSetRenderTargets(1, &renderTexture_.GetRTVHandle(), FALSE, nullptr);

    // PSOとルートシグネチャを設定
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->SetPipelineState(pso_);

    // 入力テクスチャのSRVを設定する（自分が所有するテクスチャのSRVではないため注意)
    commandList_->SetGraphicsRootDescriptorTable(0, inputGpuHandle_);

    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList_->SetGraphicsRootConstantBufferView(1, optionResource_->GetGPUVirtualAddress());
}

void BoxFilter::OnResizeBefore()
{
    renderTexture_.Reset();
}

void BoxFilter::OnResizeAfter()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "BoxFilterRenderTexture");
    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();
}

void BoxFilter::ToShaderResourceState()
{
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void BoxFilter::DebugOverlay()
{
    #ifdef _DEBUG

    bool changed = ImGui::SliderInt("Kernel Size", reinterpret_cast<int*>(&pOption_->kernelSize), 3, 99, "%d", ImGuiSliderFlags_AlwaysClamp);
    if (changed)
    {
        pOption_->kernelSize = (pOption_->kernelSize / 2) * 2 + 1;
    }

    #endif // _DEBUG
}

void BoxFilter::RegisterRootSignature()
{
    auto rsCache = RootSignatureCache::GetInstance();
    if (!rsCache->IsExist(kRootSignatureId_))
    {
        /// RootSignature作成
        RootSignatureDesc rootSignatureDesc = {};
        auto& rootParam = rootSignatureDesc.params;
        rootParam.SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam.SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_PIXEL);
        auto& staticSampler = rootSignatureDesc.staticSamplers;
        staticSampler
            .PresetPointWrap()
            .SetMaxAnisotropy(16) // 最大異方性を16に設定
            .SetShaderRegister(0) // シェーダーレジスタ番号を0に設定
            .SetRegisterSpace(0); // レジスタスペースを0に設定

        /// ルートシグネチャ記述子を登録
        rsCache->Register(kRootSignatureId_, rootSignatureDesc);
    }

    rootSignature_ = rsCache->GetOrCreate(kRootSignatureId_);
}

void BoxFilter::CreatePipelineStateObject()
{
    auto psoCache = PSOCache::GetInstance();

    /// PSOが登録されていないなら、生成するための情報を登録する
    if (!psoCache->IsExist(kPSOId_))
    {
        PSODesc desc{};
        desc.vs = kVertexShaderPath;
        desc.ps = kPixelShaderPath;
        desc.rootSignatureID = kRootSignatureId_;
        desc.blendState.Initialize(BlendDesc::BlendModes::Alpha);
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
    pso_ = psoCache->GetOrCreate(kPSOId_);
}

void BoxFilter::CreateResourceCBuffer()
{
    optionResource_ = DX12Helper::CreateBufferResource(device_, sizeof(BoxFilterOption));
    optionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    // 初期化
    pOption_->kernelSize = 3; // カーネルサイズの初期値
}