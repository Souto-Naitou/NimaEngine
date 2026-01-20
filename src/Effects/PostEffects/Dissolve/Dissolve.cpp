#include "Dissolve.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <imgui.h>
#include <Core/DirectX12/BlendDesc.h>
#include <cfloat>

void Dissolve::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;

    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "DissolveRenderTexture");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();

    // ルートシグネチャの生成
    this->RegisterRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();
}

void Dissolve::Enable(bool flag)
{
    isEnabled_ = flag;
}

void Dissolve::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    inputGpuHandle_ = gpuHandle;
}

bool Dissolve::Enabled() const
{
    return isEnabled_;
}

void Dissolve::SetTextureResource(const DX12Resource& texResource)
{
    maskTexture_ = texResource;
}

D3D12_GPU_DESCRIPTOR_HANDLE Dissolve::GetOutputTextureHandle() const
{
    return renderTexture_.GetSRVHandleGPU();
}

const std::string& Dissolve::GetName() const
{
    return name_;
}

DX12Resource* Dissolve::GetOutputResource() const
{
    return const_cast<DX12Resource*>(&renderTexture_);
}

DissolveOption& Dissolve::GetOption()
{
    return *pOption_;
}

const DissolveOption& Dissolve::GetOption() const
{
    return *pOption_;
}

void Dissolve::Apply()
{
    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void Dissolve::Finalize()
{
}

void Dissolve::Setting()
{
    this->CheckValidation();

    // レンダーテクスチャをレンダーターゲット状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // レンダーターゲットを設定 (自分が所有するテクスチャに対して設定)
    commandList_->OMSetRenderTargets(1, &renderTexture_.GetRTVHandle(), FALSE, nullptr);

    // PSOとルートシグネチャを設定
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->SetPipelineState(pso_);

    // 入力テクスチャのSRVを設定する（自分が所有するテクスチャのSRVではないため注意)
    commandList_->SetGraphicsRootDescriptorTable(0, inputGpuHandle_);
    // マスクテクスチャのSRVを設定
    commandList_->SetGraphicsRootDescriptorTable(1, maskTexture_.GetSRVHandleGPU());
    // オプションリソースのCBVを設定
    commandList_->SetGraphicsRootConstantBufferView(2, optionResource_->GetGPUVirtualAddress());

    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Dissolve::OnResizeBefore()
{
    renderTexture_.Reset();
}

void Dissolve::OnResizeAfter()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "DissolveRenderTexture");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();
}

void Dissolve::ToShaderResourceState()
{
    // レンダーテクスチャをシェーダーリソース状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Dissolve::DebugOverlay()
{
    #ifdef _DEBUG

    ImGui::SliderFloat("Threshold", &pOption_->threshold, FLT_MIN, 1.0f, "%.2f");
    ImGui::SliderFloat("Edge threshold offset", &pOption_->edgeThresholdOffset, FLT_MIN, 1.0f, "%.2f");
    ImGui::ColorEdit4("DissolveColor", &pOption_->colorDissolve.x);
    ImGui::ColorEdit4("EdgeColor", &pOption_->colorEdge.x);

    #endif // _DEBUG
}

void Dissolve::RegisterRootSignature()
{
    auto rsCache = RootSignatureCache::GetInstance();
    if (!rsCache->IsExist(kRootSignatureId_))
    {
        /// RootSignature作成
        RootSignatureDesc rootSignatureDesc = {};
        auto& rootParam = rootSignatureDesc.params;
        rootParam.SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam.SetParameter(1, "t1", D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam.SetParameter(2, "b0", D3D12_SHADER_VISIBILITY_PIXEL);
        auto& staticSampler = rootSignatureDesc.staticSamplers;
        staticSampler
            .PresetPointWrap()
            .SetMaxAnisotropy(16)
            .SetShaderRegister(0)
            .SetRegisterSpace(0);

        /// ルートシグネチャ記述子を登録
        rsCache->Register(kRootSignatureId_, rootSignatureDesc);
    }

    rootSignature_ = rsCache->GetOrCreate(kRootSignatureId_);
}

void Dissolve::CreatePipelineStateObject()
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
        rasterizerDesc.MultisampleEnable = TRUE;
        rasterizerDesc.AntialiasedLineEnable = TRUE;
        desc.inputLayoutDesc.pInputElementDescs = nullptr;
        desc.inputLayoutDesc.NumElements = 0;
        psoCache->Register(kPSOId_, desc);
    }

    /// 生成もしくは生成済みを取得する
    pso_ = psoCache->GetOrCreate(kPSOId_);
}

void Dissolve::CreateResourceCBuffer()
{
    optionResource_ = DX12Helper::CreateBufferResource(device_, sizeof(DissolveOption));
    optionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    // 初期化
    pOption_->threshold = 0.5f; // デフォルトのしきい値を設定
    pOption_->edgeThresholdOffset = 0.1f; // エッジのしきい値オフセットを設定
    pOption_->colorDissolve = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルトの色を設定
    pOption_->colorEdge = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルトの色を設定
}

void Dissolve::CheckValidation() const
{
    if (optionResource_ == nullptr)
    {
        Logger::GetInstance()->LogError(
            __FILE__,
            __FUNCTION__,
            "Option resource is not initialized."
        );
        assert(false);
    }
    if (maskTexture_.GetSRVHandleGPU().ptr == 0)
    {
        Logger::GetInstance()->LogError(
            __FILE__,
            __FUNCTION__,
            "Mask texture is not set."
        );
        assert(false);
    }
    if (inputGpuHandle_.ptr == 0)
    {
        Logger::GetInstance()->LogError(
            __FILE__,
            __FUNCTION__,
            "Input texture handle is not set."
        );
        assert(false);
    }
    if (renderTexture_.GetResource() == nullptr)
    {
        Logger::GetInstance()->LogError(
            __FILE__,
            __FUNCTION__,
            "Render texture is not initialized."
        );
        assert(false);
    }
    if (pso_ == nullptr)
    {
        Logger::GetInstance()->LogError(
            __FILE__,
            __FUNCTION__,
            "Pipeline state object is not initialized."
        );
        assert(false);
    }
    if (rootSignature_ == nullptr)
    {
        Logger::GetInstance()->LogError(
            __FILE__,
            __FUNCTION__,
            "Root signature is not initialized."
        );
        assert(false);
    }
}
