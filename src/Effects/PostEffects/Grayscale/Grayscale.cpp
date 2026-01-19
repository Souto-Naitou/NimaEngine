#include "Grayscale.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/DirectX12/BlendDesc.h>
#include <config/EngineSetting.h>
#include <imgui.h>

void Grayscale::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;
    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_Grayscale");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();

    // ルートシグネチャの生成
    this->RegisterRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 定数バッファリソースの生成
    this->CreateResorceCBuffer();
}

void Grayscale::Enable(bool flag)
{
    isEnabled_ = flag;
}

void Grayscale::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    inputGpuHandle_ = gpuHandle;
}

bool Grayscale::Enabled() const
{
    return isEnabled_;
}

D3D12_GPU_DESCRIPTOR_HANDLE Grayscale::GetOutputTextureHandle() const
{
    return renderTexture_.GetSRVHandleGPU();
}

const std::string& Grayscale::GetName() const
{
    return name_;
}

DX12Resource* Grayscale::GetOutputResource() const
{
    return const_cast<DX12Resource*>(&renderTexture_);
}

void Grayscale::Apply()
{
    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void Grayscale::Finalize()
{
}

void Grayscale::Setting()
{
    // レンダーテクスチャをレンダーターゲット状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // クリア
    commandList_->ClearRenderTargetView(renderTexture_.GetRTVHandle(), &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    // レンダーターゲットを設定 (自分が所有するテクスチャに対して設定)
    commandList_->OMSetRenderTargets(1, &renderTexture_.GetRTVHandle(), FALSE, nullptr);

    // PSOとルートシグネチャを設定
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->SetPipelineState(pso_);

    // 入力テクスチャのSRVを設定する（自分が所有するテクスチャのSRVではないため注意)
    commandList_->SetGraphicsRootDescriptorTable(0, inputGpuHandle_);
    // オプション用CBVを設定
    commandList_->SetGraphicsRootConstantBufferView(1, optionResource_->GetGPUVirtualAddress());

    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Grayscale::OnResizeBefore()
{
    renderTexture_.Reset();
}

void Grayscale::OnResizeAfter()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_Grayscale");
    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();
}

void Grayscale::ToShaderResourceState()
{
    // レンダーテクスチャをシェーダーリソース状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void Grayscale::DebugOverlay()
{
#ifdef _DEBUG
    ImGui::DragFloat("Power", &pOption_->power, 0.01f);
#endif // _DEBUG
}

void Grayscale::RegisterRootSignature()
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

void Grayscale::CreatePipelineStateObject()
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

void Grayscale::CreateResorceCBuffer()
{
    optionResource_ = DX12Helper::CreateBufferResource(device_, sizeof(Grayscale));
    optionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    pOption_->power = 1.0f;
}
