#include "DepthBasedOutline.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Window/Window.h>
#include <imgui.h>
#include <Core/DirectX12/BlendDesc.h>
#include <Core/DirectX12/StaticSamplerDesc/StaticSamplerDesc.h>
#include <Core/DirectX12/RootParameters/RootParameters.h>
#include <Core/DirectX12/PipelineStateObject/PipelineStateObject.h>
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
    this->CreateRootSignature();

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
    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->SetPipelineState(pso_.GetPSO());

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

void DepthBasedOutline::CreateRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    RootParameters<4> rootParameters = {};
    rootParameters
        .SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(2, "b1", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(3, "t1", D3D12_SHADER_VISIBILITY_PIXEL);

    descriptionRootSignature.pParameters = rootParameters.GetParams();
    descriptionRootSignature.NumParameters = rootParameters.GetSize();

    StaticSamplerDesc staticSampler = {};
    staticSampler
        .PresetPointClamp()
        .SetMaxAnisotropy(16)
        .SetShaderRegister(0)
        .SetRegisterSpace(0);

    descriptionRootSignature.pStaticSamplers = &staticSampler.Get();
    descriptionRootSignature.NumStaticSamplers = 1;

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = device_->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void DepthBasedOutline::CreatePipelineStateObject()
{
    IDxcUtils* dxcUtils = pDx12_->GetDxcUtils();
    IDxcCompiler3* dxcCompiler = pDx12_->GetDxcCompiler();
    IDxcIncludeHandler* includeHandler = pDx12_->GetIncludeHandler();

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = nullptr;
    inputLayoutDesc.NumElements = 0;

    BlendDesc blendDesc = {};
    blendDesc.Initialize(BlendDesc::BlendModes::Test);

    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.MultisampleEnable = TRUE;
    rasterizerDesc.AntialiasedLineEnable = TRUE;

    vertexShaderBlob_ = DX12Helper::CompileShader(kVertexShaderPath, L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vertexShaderBlob_ != nullptr);
    pixelShaderBlob_ = DX12Helper::CompileShader(kPixelShaderPath, L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(pixelShaderBlob_ != nullptr);

    try
    {
        pso_.SetRootSignature(rootSignature_.Get())
            .SetInputLayout(inputLayoutDesc)
            .SetVertexShader(vertexShaderBlob_.Get()->GetBufferPointer(), vertexShaderBlob_.Get()->GetBufferSize())
            .SetPixelShader(pixelShaderBlob_.Get()->GetBufferPointer(), pixelShaderBlob_.Get()->GetBufferSize())
            .SetBlendState(blendDesc.Get())
            .SetRasterizerState(rasterizerDesc)
            .SetRenderTargetFormats(1, &renderTexture_.GetStateTracker().GetFormat(), DXGI_FORMAT_D24_UNORM_S8_UINT)
            .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
            .SetSampleDesc({ 1, 0 })
            .SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK)
            .Build(device_);
    }
    catch (const std::exception& _e)
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, _e.what());
        assert(false);
    }
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