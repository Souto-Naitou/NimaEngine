#include "PrewittOutline.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <imgui.h>
#include <Core/DirectX12/BlendDesc.h>

void PrewittOutline::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;
    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_PrewittOutline");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();

    // ルートシグネチャの生成
    this->RegisterRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();
}

void PrewittOutline::Enable(bool flag)
{
    isEnabled_ = flag;
}

void PrewittOutline::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    inputGpuHandle_ = gpuHandle;
}

bool PrewittOutline::Enabled() const
{
    return isEnabled_;
}

D3D12_GPU_DESCRIPTOR_HANDLE PrewittOutline::GetOutputTextureHandle() const
{
    return renderTexture_.GetSRVHandleGPU();
}

const std::string& PrewittOutline::GetName() const
{
    return name_;
}

DX12Resource* PrewittOutline::GetOutputResource() const
{
    return const_cast<DX12Resource*>(&renderTexture_);
}

PrewittOutlineOption& PrewittOutline::GetOption()
{
    return *pOption_;
}

const PrewittOutlineOption& PrewittOutline::GetOption() const
{
    return *pOption_;
}

void PrewittOutline::Apply()
{
    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void PrewittOutline::Finalize()
{
}

void PrewittOutline::Setting()
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

void PrewittOutline::OnResizeBefore()
{
    renderTexture_.Reset();
}void PrewittOutline::OnResizeAfter()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_PrewittOutline");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();
}

void PrewittOutline::ToShaderResourceState()
{
    // レンダーテクスチャをシェーダーリソース状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void PrewittOutline::DebugOverlay()
{
    #ifdef _DEBUG

    ImGui::DragFloat("Weight Multiply", &pOption_->weightMultiply, 0.01f, 0.1f, 0.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

    #endif // _DEBUG
}

void PrewittOutline::RegisterRootSignature()
{
    auto rsCache = RootSignatureCache::GetInstance();
    if (!rsCache->IsExist(kRootSignatureId_))
    {
        RootSignatureDesc rootSignatureDesc = {};
        auto& rootParam = rootSignatureDesc.params;
        rootParam.SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam.SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_PIXEL);
        auto& staticSampler = rootSignatureDesc.staticSamplers;
        staticSampler
            .PresetPointWrap()
            .SetMaxAnisotropy(16)
            .SetShaderRegister(0)
            .SetRegisterSpace(0);
        rsCache->Register(kRootSignatureId_, rootSignatureDesc);
    }
    rootSignature_ = rsCache->GetOrCreate(kRootSignatureId_);
}

void PrewittOutline::CreatePipelineStateObject()
{
    auto psoCache = PSOCache::GetInstance();
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
    pso_ = psoCache->GetOrCreate(kPSOId_);
}

void PrewittOutline::CreateResourceCBuffer()
{
    optionResource_ = DX12Helper::CreateBufferResource(device_, sizeof(PrewittOutlineOption));
    optionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    // 初期化
    pOption_->weightMultiply = 6.0f;
}