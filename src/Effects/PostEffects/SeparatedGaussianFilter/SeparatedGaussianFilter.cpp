#include "SeparatedGaussianFilter.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/DirectX12/RootParameters/RootParameters.h>
#include <Core/DirectX12/StaticSamplerDesc/StaticSamplerDesc.h>
#include <Core/DirectX12/BlendDesc.h>
#include <Core/DirectX12/PipelineStateObject/PSOBuilder.h>
#include <imgui.h>
#include <Math/Functions.hpp>
#include <config/EngineSetting.h>

void SeparatedGaussianFilter::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;
    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, horizontalGaussTexture_, kNameHorizontal);
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, kNameVertical);

    // レンダーテクスチャのSRVを生成
    horizontalGaussTexture_.CreateSRV();
    renderTexture_.CreateSRV();

    // ルートシグネチャの生成
    this->CreateRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();
}

void SeparatedGaussianFilter::Enable(bool flag)
{
    isEnabled_ = flag;
}

void SeparatedGaussianFilter::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    inputGpuHandle_ = gpuHandle;
}

void SeparatedGaussianFilter::SetSigma(float sigma)
{
    sigma_ = sigma;
    this->CreateKernel();
}

bool SeparatedGaussianFilter::Enabled() const
{
    return isEnabled_;
}

D3D12_GPU_DESCRIPTOR_HANDLE SeparatedGaussianFilter::GetOutputTextureHandle() const
{
    return renderTexture_.GetSRVHandleGPU();
}

const std::string& SeparatedGaussianFilter::GetName() const
{
    return name_;
}

DX12Resource* SeparatedGaussianFilter::GetOutputResource() const
{
    return const_cast<DX12Resource*>(&renderTexture_);
}

SeparatedGaussianFilterOption& SeparatedGaussianFilter::GetOption()
{
    return *pOption_;
}

const SeparatedGaussianFilterOption& SeparatedGaussianFilter::GetOption() const
{
    return *pOption_;
}

void SeparatedGaussianFilter::Apply()
{
    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画

    // 縦方向のガウスフィルタを適用
    horizontalGaussTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    this->PreDrawSetting(horizontalGaussTexture_.GetSRVHandleGPU(), renderTexture_.GetRTVHandle(), execInfoResourceVertical_.Get());

    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void SeparatedGaussianFilter::Finalize()
{
}

void SeparatedGaussianFilter::Setting()
{
    horizontalGaussTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_RENDER_TARGET);
    this->PreDrawSetting(inputGpuHandle_, horizontalGaussTexture_.GetRTVHandle(), execInfoResourceHorizontal_.Get());
}

void SeparatedGaussianFilter::OnResizeBefore()
{
    horizontalGaussTexture_.Reset();
    renderTexture_.Reset();
}

void SeparatedGaussianFilter::OnResizeAfter()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, horizontalGaussTexture_, kNameHorizontal);
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, kNameVertical);

    // レンダーテクスチャのSRVを生成
    horizontalGaussTexture_.CreateSRV();
    renderTexture_.CreateSRV();
}

void SeparatedGaussianFilter::ToShaderResourceState()
{
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void SeparatedGaussianFilter::DebugOverlay()
{
    #ifdef _DEBUG

    bool isChanged = false;

    if (ImGui::SliderInt("Kernel Size", reinterpret_cast<int*>(&pOption_->kernelSize), 3, 31, "%d", ImGuiSliderFlags_AlwaysClamp))
    {
        pOption_->kernelSize = (pOption_->kernelSize / 2) * 2 + 1;
        isChanged = true;
    }

    if (ImGui::DragFloat("Sigma", &sigma_, 0.01f, 0.1f, 0.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
    {
        isChanged = true;
    }

    if (isChanged)
    {
        this->CreateKernel();
    }

    #endif // _DEBUG
}

void SeparatedGaussianFilter::CreateRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    RootParameters<3> rootParameters = {};
    rootParameters
        .SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(2, "b1", D3D12_SHADER_VISIBILITY_PIXEL);

    descriptionRootSignature.pParameters = rootParameters.GetParams();
    descriptionRootSignature.NumParameters = rootParameters.GetSize();

    StaticSamplerDesc staticSamplerDesc = {};
    staticSamplerDesc
        .PresetPointClamp()
        .SetMaxAnisotropy(16)
        .SetShaderRegister(0)
        .SetRegisterSpace(0);

    descriptionRootSignature.pStaticSamplers = &staticSamplerDesc.Get();
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

void SeparatedGaussianFilter::CreatePipelineStateObject()
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
        PSOBuilder psoBuilder;
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

void SeparatedGaussianFilter::CreateResourceCBuffer()
{
    optionResource_ = DX12Helper::CreateBufferResource(device_, sizeof(SeparatedGaussianFilterOption));
    optionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    // 縦と横それぞれの実行情報を格納するリソースを作成
    execInfoResourceHorizontal_ = DX12Helper::CreateBufferResource(device_, sizeof(SeparatedGaussianFilterExecInfo));
    execInfoResourceHorizontal_->Map(0, nullptr, reinterpret_cast<void**>(&pExecInfoHorizontal_));
    execInfoResourceVertical_ = DX12Helper::CreateBufferResource(device_, sizeof(SeparatedGaussianFilterExecInfo));
    execInfoResourceVertical_->Map(0, nullptr, reinterpret_cast<void**>(&pExecInfoVertical_));

    // 初期化
    pOption_->kernelSize = 3; // カーネルサイズの初期値

    pExecInfoHorizontal_->direction[0] = 1;
    pExecInfoHorizontal_->direction[1] = 0;

    pExecInfoVertical_->direction[0] = 0; // 垂直方向
    pExecInfoVertical_->direction[1] = 1; // 垂直方向

    this->CreateKernel();
}

void SeparatedGaussianFilter::PreDrawSetting(D3D12_GPU_DESCRIPTOR_HANDLE inputGpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE outputCpuHandle, ID3D12Resource* execInfoResource)
{
    // クリア
    commandList_->ClearRenderTargetView(outputCpuHandle, &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    // レンダーターゲットを設定 (自分が所有するテクスチャに対して設定)
    commandList_->OMSetRenderTargets(1, &outputCpuHandle, FALSE, nullptr);

    // PSOとルートシグネチャを設定
    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->SetPipelineState(pso_.Get());

    // 入力テクスチャのSRVを設定する（自分が所有するテクスチャのSRVではないため注意)
    commandList_->SetGraphicsRootDescriptorTable(0, inputGpuHandle);

    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList_->SetGraphicsRootConstantBufferView(1, optionResource_->GetGPUVirtualAddress());
    commandList_->SetGraphicsRootConstantBufferView(2, execInfoResource->GetGPUVirtualAddress());
}

void SeparatedGaussianFilter::CreateKernel()
{
    // カーネルサイズに応じて重みを計算
    int kernelSize = pOption_->kernelSize;
    int halfKernelSize = kernelSize / 2;
    float sum = 0.0f;
    for (int i = 0; i < kernelSize; ++i)
    {
        int x = i - halfKernelSize;
        // 1次元のガウス関数
        float w = std::exp(-0.5f * (x * x) / (sigma_ * sigma_));
        pOption_->weights[i].value = w;
        sum += w;
    }

    // 正規化（合計が1になるように）
    for (int i = 0; i < kernelSize; ++i)
    {
        pOption_->weights[i].value /= sum;
    }
}
