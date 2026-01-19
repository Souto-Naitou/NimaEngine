#include "RadialBlur.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <imgui.h>
#include <Core/DirectX12/BlendDesc.h>
#include <config/EngineSetting.h>

void RadialBlur::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;
    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_RadialBlur");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();

    // ルートシグネチャの生成
    this->RegisterRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();
}

void RadialBlur::Enable(bool flag)
{
    isEnabled_ = flag;
}

void RadialBlur::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    inputGpuHandle_ = gpuHandle;
}

bool RadialBlur::Enabled() const
{
    return isEnabled_;
}

D3D12_GPU_DESCRIPTOR_HANDLE RadialBlur::GetOutputTextureHandle() const
{
    return renderTexture_.GetSRVHandleGPU();
}

const std::string& RadialBlur::GetName() const
{
    return name_;
}

DX12Resource* RadialBlur::GetOutputResource() const
{
    return const_cast<DX12Resource*>(&renderTexture_);
}

RadialBlurOption& RadialBlur::GetOption()
{
    return *pOption_;
}

const RadialBlurOption& RadialBlur::GetOption() const
{
    return *pOption_;
}

void RadialBlur::Apply()
{
    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void RadialBlur::Finalize()
{
}

void RadialBlur::Setting()
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

    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList_->SetGraphicsRootConstantBufferView(1, optionResource_->GetGPUVirtualAddress());
}

void RadialBlur::OnResizeBefore()
{
    renderTexture_.GetResource().Reset();
}

void RadialBlur::OnResizeAfter()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_RadialBlur");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();
}

void RadialBlur::ToShaderResourceState()
{
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void RadialBlur::DebugOverlay()
{
    #ifdef _DEBUG

    ImGui::DragFloat2("Center", &pOption_->center.x, 0.01f, FLT_MIN);
    ImGui::InputInt("Samples", &pOption_->samples, 1, 2);
    ImGui::DragFloat("BlurWidth", &pOption_->blurWidth, 0.0001f);

    #endif //_DEBUG
}

void RadialBlur::RegisterRootSignature()
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
            .PresetPointClamp()
            .SetMaxAnisotropy(16)
            .SetShaderRegister(0)
            .SetRegisterSpace(0);
        rsCache->Register(kRootSignatureId_, rootSignatureDesc);
    }
    rootSignature_ = rsCache->GetOrCreate(kRootSignatureId_);
}

void RadialBlur::CreatePipelineStateObject()
{
    auto psoCache = PSOCache::GetInstance();
    if (!psoCache->IsExist(kPSOId_))
    {
        PSODesc desc{};
        desc.vs = kVertexShaderPath;
        desc.ps = kPixelShaderPath;
        desc.rootSignatureID = kRootSignatureId_;
        desc.blendState.Initialize(BlendDesc::BlendModes::Test);
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

void RadialBlur::CreateResourceCBuffer()
{
    optionResource_ = DX12Helper::CreateBufferResource(device_, sizeof(RadialBlurOption));
    optionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    pOption_->center = { 0.5f, 0.5f };
    pOption_->samples = 4;
    pOption_->blurWidth = 0.01f;
}
