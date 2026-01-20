#include "DepthBasedOutline.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Window/Window.h>
#include <imgui.h>
#include <Core/DirectX12/BlendDesc.h>
#include <config/EngineSetting.h>

void DepthBasedOutline::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;
    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_DepthBasedOutline");

    // レンダーテクスチャのSRVを生成
    this->CreateSRV();

    // ルートシグネチャの生成
    this->RegisterRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();
}

void DepthBasedOutline::Enable(bool flag)
{
    isEnabled_ = flag;
}

void DepthBasedOutline::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    inputGpuHandle_ = gpuHandle;
}

bool DepthBasedOutline::Enabled() const
{
    return isEnabled_;
}

D3D12_GPU_DESCRIPTOR_HANDLE DepthBasedOutline::GetOutputTextureHandle() const
{
    return renderTexture_.GetSRVHandleGPU();
}

const std::string& DepthBasedOutline::GetName() const
{
    return name_;
}

DX12Resource* DepthBasedOutline::GetOutputResource() const
{
    return const_cast<DX12Resource*>(&renderTexture_);
}

DepthBasedOutlineOption& DepthBasedOutline::GetOption()
{
    return *pOption_;
}

const DepthBasedOutlineOption& DepthBasedOutline::GetOption() const
{
    return *pOption_;
}

DepthBasedOutlineMaterial& DepthBasedOutline::GetMaterial()
{
    return *pMaterial_;
}

const DepthBasedOutlineMaterial& DepthBasedOutline::GetMaterial() const
{
    return *pMaterial_;
}

void DepthBasedOutline::Apply()
{
    auto dsResource = pDx12_->GetDepthStencilResource();
    dsResource->GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
    dsResource->GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

void DepthBasedOutline::Finalize()
{
}

void DepthBasedOutline::Setting()
{
    // レンダーテクスチャをレンダーターゲット状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_RENDER_TARGET);

    commandList_->ClearRenderTargetView(renderTexture_.GetRTVHandle(), &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    // レンダーターゲットを設定 (自分が所有するテクスチャに対して設定)
    commandList_->OMSetRenderTargets(1, &renderTexture_.GetRTVHandle(), FALSE, nullptr);

    // PSOとルートシグネチャを設定
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->SetPipelineState(pso_);

    // 入力テクスチャのSRVを設定する（自分が所有するテクスチャのSRVではないため注意)
    commandList_->SetGraphicsRootDescriptorTable(0, inputGpuHandle_);

    commandList_->SetGraphicsRootConstantBufferView(1, optionResource_->GetGPUVirtualAddress());

    commandList_->SetGraphicsRootConstantBufferView(2, materialResource_->GetGPUVirtualAddress());

    commandList_->SetGraphicsRootDescriptorTable(3, depthGpuHandle_);

    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DepthBasedOutline::OnResizeBefore()
{
    SRVManager::GetInstance()->Deallocate(srvIndexDepth_);
    renderTexture_.Reset();
}

void DepthBasedOutline::OnResizeAfter()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_DepthBasedOutline");

    // レンダーテクスチャのSRVを生成
    this->CreateSRV();
}

void DepthBasedOutline::ToShaderResourceState()
{
    // 完成したレンダーテクスチャをシェーダーリソース状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void DepthBasedOutline::DebugOverlay()
{
    #ifdef _DEBUG

    ImGui::DragFloat("Weight Multiply", &pOption_->weightMultiply, 0.01f, 0.1f, 0.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

    #endif // _DEBUG
}

void DepthBasedOutline::RegisterRootSignature()
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
        rootParam.SetParameter(3, "t1", D3D12_SHADER_VISIBILITY_PIXEL);
        auto& staticSampler = rootSignatureDesc.staticSamplers;
        staticSampler
            .PresetPointClamp()
            .SetMaxAnisotropy(16)
            .SetShaderRegister(0)
            .SetRegisterSpace(0);

        /// ルートシグネチャ記述子を登録
        rsCache->Register(kRootSignatureId_, rootSignatureDesc);
    }

    rootSignature_ = rsCache->GetOrCreate(kRootSignatureId_);
}

void DepthBasedOutline::CreatePipelineStateObject()
{
    auto psoCache = PSOCache::GetInstance();

    /// PSOが登録されていないなら、生成するための情報を登録する
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

    /// 生成もしくは生成済みを取得する
    pso_ = psoCache->GetOrCreate(kPSOId_);
}

void DepthBasedOutline::CreateResourceCBuffer()
{
    optionResource_ = DX12Helper::CreateBufferResource(device_, sizeof(DepthBasedOutlineOption));
    materialResource_ = DX12Helper::CreateBufferResource(device_, sizeof(DepthBasedOutlineMaterial));
    optionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&pMaterial_));

    // 初期化
    pOption_->weightMultiply = 1.0f;
    auto projection = Matrix4x4::PerspectiveFovMatrix(0.45f, static_cast<float>(Window::clientWidth) / static_cast<float>(Window::clientHeight), 0.1f, 1000.0f);
    pMaterial_->projectionInverse = projection.Inverse();
}

void DepthBasedOutline::CreateSRV()
{
    auto sm = SRVManager::GetInstance();

    srvIndexDepth_ = sm->Allocate();
    sm->CreateForTexture2D(srvIndexDepth_, pDx12_->GetDepthStencilResource()->GetResource().Get(), DXGI_FORMAT_R24_UNORM_X8_TYPELESS, 1);
    depthGpuHandle_ = sm->GetGPUDescriptorHandle(srvIndexDepth_);
    renderTexture_.CreateSRV();
}