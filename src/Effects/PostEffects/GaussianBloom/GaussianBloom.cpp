#include "GaussianBloom.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/DirectX12/BlendDesc.h>
#include <imgui.h>
#include <config/EngineSetting.h>

void GaussianBloom::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;

    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, outputTexture_, "GaussianBloomRenderTexture");

    // レンダーテクスチャのSRVを生成
    outputTexture_.CreateSRV();

    // ルートシグネチャの生成
    this->RegisterRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();
    
    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();

    // 内部エフェクトの初期化
    this->InitializeLuminanceOutputFilter();
    this->InitializeSeparatedGaussianFilter();
}

void GaussianBloom::Enable(bool flag)
{
    isEnabled_ = flag;
}

void GaussianBloom::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    inputGpuHandle_ = gpuHandle;
}

bool GaussianBloom::Enabled() const
{
    return isEnabled_;
}

D3D12_GPU_DESCRIPTOR_HANDLE GaussianBloom::GetOutputTextureHandle() const
{
    return outputTexture_.GetSRVHandleGPU();
}

const std::string& GaussianBloom::GetName() const
{
    return name_;
}

DX12Resource* GaussianBloom::GetOutputResource() const
{
    return const_cast<DX12Resource*>(&outputTexture_);
}

GaussianBloomOption& GaussianBloom::GetOption()
{
    return *cbOptionData_;
}

const GaussianBloomOption& GaussianBloom::GetOption() const
{
    return *cbOptionData_;
}

SeparatedGaussianFilter* GaussianBloom::GetSeparatedGaussianFilter()
{
    return pSeparatedGaussianFilter_.get();
}

const SeparatedGaussianFilter* GaussianBloom::GetSeparatedGaussianFilter() const
{
    return pSeparatedGaussianFilter_.get();
}

LuminanceOutput* GaussianBloom::GetLuminanceOutputFilter()
{
    return pLuminanceOutput_.get();
}

const LuminanceOutput* GaussianBloom::GetLuminanceOutputFilter() const
{
    return pLuminanceOutput_.get();
}

void GaussianBloom::SetKernelSize(int size)
{
    pSeparatedGaussianFilter_->GetOption().kernelSize = size;
}

void GaussianBloom::SetSigma(float sigma)
{
    pSeparatedGaussianFilter_->SetSigma(sigma);
}

void GaussianBloom::SetThreshold(float threshold)
{
    pLuminanceOutput_->GetOption().threshold = threshold;
}

void GaussianBloom::SetBloomIntensity(float intensity)
{
    cbOptionData_->bloomIntensity = intensity;
}

void GaussianBloom::Apply()
{
    D3D12_GPU_DESCRIPTOR_HANDLE outputHandleLuminance = {};
    outputHandleLuminance = this->ApplyFilter(inputGpuHandle_, pLuminanceOutput_.get());
    D3D12_GPU_DESCRIPTOR_HANDLE outputHandleGaussian = {};
    outputHandleGaussian = this->ApplyFilter(outputHandleLuminance, pSeparatedGaussianFilter_.get());

    outputTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    this->PreDrawSetting(outputHandleGaussian, outputTexture_.GetRTVHandle());
    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void GaussianBloom::Finalize()
{
}

void GaussianBloom::Setting()
{
}

void GaussianBloom::OnResizeBefore()
{
    outputTexture_.Reset();

    // 内部エフェクトのリサイズ
    pLuminanceOutput_->OnResizeBefore();
    pSeparatedGaussianFilter_->OnResizeBefore();
}

void GaussianBloom::OnResizeAfter()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, outputTexture_, "GaussianBloomRenderTexture");

    // レンダーテクスチャのSRVを生成
    outputTexture_.CreateSRV();

    // 内部エフェクトのリサイズ
    pLuminanceOutput_->OnResizeAfter();
    pSeparatedGaussianFilter_->OnResizeAfter();
}

void GaussianBloom::ToShaderResourceState()
{
    outputTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void GaussianBloom::DebugOverlay()
{
    #ifdef _DEBUG

    pSeparatedGaussianFilter_->DebugOverlay();
    pLuminanceOutput_->DebugOverlay();
    ImGui::DragFloat("Bloom Intensity", &cbOptionData_->bloomIntensity, 0.01f, FLT_MIN, 0.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

    #endif // _DEBUG
}

void GaussianBloom::RegisterRootSignature()
{
    auto rsCache = RootSignatureCache::GetInstance();
    if (!rsCache->IsExist(kRootSignatureId_))
    {
        RootSignatureDesc rootSignatureDesc = {};
        auto& rootParam = rootSignatureDesc.params;
        rootParam.SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam.SetParameter(1, "t1", D3D12_SHADER_VISIBILITY_PIXEL);
        rootParam.SetParameter(2, "b0", D3D12_SHADER_VISIBILITY_PIXEL);
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

void GaussianBloom::CreatePipelineStateObject()
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

void GaussianBloom::PreDrawSetting(D3D12_GPU_DESCRIPTOR_HANDLE inputGpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE outputCpuHandle)
{
    // クリア
    commandList_->ClearRenderTargetView(outputCpuHandle, &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    // レンダーターゲットを設定 (自分が所有するテクスチャに対して設定)
    commandList_->OMSetRenderTargets(1, &outputCpuHandle, FALSE, nullptr);

    // PSOとルートシグネチャを設定
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->SetPipelineState(pso_);

    // 入力テクスチャのSRVを設定する（自分が所有するテクスチャのSRVではないため注意)
    commandList_->SetGraphicsRootDescriptorTable(0, inputGpuHandle_);
    commandList_->SetGraphicsRootDescriptorTable(1, inputGpuHandle);
    commandList_->SetGraphicsRootConstantBufferView(2, cbOptionResorce_->GetGPUVirtualAddress());

    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void GaussianBloom::InitializeLuminanceOutputFilter()
{
    // LuminanceOutputFilterの初期化
    pLuminanceOutput_ = std::make_unique<LuminanceOutput>();
    pLuminanceOutput_->Initialize({pDx12_, commandList_});
    pLuminanceOutput_->Enable(true);
}

void GaussianBloom::InitializeSeparatedGaussianFilter()
{
    // SeparatedGaussianFilterの初期化
    pSeparatedGaussianFilter_ = std::make_unique<SeparatedGaussianFilter>();
    pSeparatedGaussianFilter_->Initialize({pDx12_, commandList_});
    pSeparatedGaussianFilter_->Enable(true);
}

D3D12_GPU_DESCRIPTOR_HANDLE GaussianBloom::ApplyFilter(D3D12_GPU_DESCRIPTOR_HANDLE inputGpuHandle, IPostEffect* pEffect)
{
    pEffect->SetInputTextureHandle(inputGpuHandle);
    pEffect->Setting();
    pEffect->Apply();
    pEffect->ToShaderResourceState();
    return pEffect->GetOutputTextureHandle();
}

void GaussianBloom::CreateResourceCBuffer()
{
    cbOptionResorce_ = DX12Helper::CreateBufferResource(device_, sizeof(GaussianBloomOption));
    cbOptionResorce_->Map(0, nullptr, reinterpret_cast<void**>(&cbOptionData_));

    cbOptionData_->bloomIntensity = 1.0f;
}