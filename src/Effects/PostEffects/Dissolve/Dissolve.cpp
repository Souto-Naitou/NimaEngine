#include "Dissolve.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <imgui.h>
#include <Core/DirectX12/BlendDesc.h>
#include <Core/DirectX12/StaticSamplerDesc/StaticSamplerDesc.h>
#include <Core/DirectX12/RootParameters/RootParameters.h>
#include <Core/DirectX12/PipelineStateObject/PipelineStateObject.h>
#include <cfloat>

void Dissolve::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;

    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "DissolveRenderTexture");

    // レンダーテクスチャのSRVを生成
    Helper::CreateSRV(renderTexture_);

    // ルートシグネチャの生成
    this->CreateRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();
}

void Dissolve::Enable(bool _flag)
{
    isEnabled_ = _flag;
}

void Dissolve::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle)
{
    inputGpuHandle_ = _gpuHandle;
}

bool Dissolve::Enabled() const
{
    return isEnabled_;
}

void Dissolve::SetTextureResource(const DX12Resource& _texResource)
{
    maskTexture_ = _texResource;
}

D3D12_GPU_DESCRIPTOR_HANDLE Dissolve::GetOutputTextureHandle() const
{
    return rtvHandleGpu_;
}

const std::string& Dissolve::GetName() const
{
    return name_;
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
    commandList_->OMSetRenderTargets(1, &rtvHandleCpu_, FALSE, nullptr);

    // PSOとルートシグネチャを設定
    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->SetPipelineState(pso_.Get());

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
    SRVManager::GetInstance()->Deallocate(srvHeapIndex_);
    renderTexture_.GetResource().Reset();
        renderTexture_.GetStateTracker().Reset();
}

void Dissolve::OnResizedBuffers()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "DissolveRenderTexture");

    // レンダーテクスチャのSRVを生成
    Helper::CreateSRV(renderTexture_);
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

void Dissolve::CreateRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    RootParameters<3> rootParameters = {};
    rootParameters
        .SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(1, "t1", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(2, "b0", D3D12_SHADER_VISIBILITY_PIXEL);

    descriptionRootSignature.pParameters = rootParameters.GetParams();
    descriptionRootSignature.NumParameters = rootParameters.GetSize();

    StaticSamplerDesc staticSampler = {};
    staticSampler
        .PresetPointWrap()
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

void Dissolve::CreatePipelineStateObject()
{
    IDxcUtils* dxcUtils = pDx12_->GetDxcUtils();
    IDxcCompiler3* dxcCompiler = pDx12_->GetDxcCompiler();
    IDxcIncludeHandler* includeHandler = pDx12_->GetIncludeHandler();

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = nullptr;
    inputLayoutDesc.NumElements = 0;

    BlendDesc blendDesc = {};
    blendDesc.Initialize(BlendDesc::BlendModes::Alpha);

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
        PipelineStateObject psoBuilder;
        psoBuilder.SetRootSignature(rootSignature_.Get())
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
        pso_ = psoBuilder.GetPSO();
    }
    catch (const std::exception& _e)
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, _e.what());
        assert(false);
    }
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
