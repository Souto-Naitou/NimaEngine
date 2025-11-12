#include "GaussianBloom.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/DirectX12/RootParameters/RootParameters.h>
#include <Core/DirectX12/StaticSamplerDesc/StaticSamplerDesc.h>
#include <Core/DirectX12/BlendDesc.h>
#include <Core/DirectX12/PipelineStateObject/PipelineStateObject.h>
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
    this->CreateRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();
    
    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();

    // 内部エフェクトの初期化
    this->InitializeLuminanceOutputFilter();
    this->InitializeSeparatedGaussianFilter();
}

void GaussianBloom::Enable(bool _flag)
{
    isEnabled_ = _flag;
}

void GaussianBloom::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle)
{
    inputGpuHandle_ = _gpuHandle;
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

void GaussianBloom::SetKernelSize(int _size)
{
    pSeparatedGaussianFilter_->GetOption().kernelSize = _size;
}

void GaussianBloom::SetSigma(float _sigma)
{
    pSeparatedGaussianFilter_->SetSigma(_sigma);
}

void GaussianBloom::SetThreshold(float _threshold)
{
    pLuminanceOutput_->GetOption().threshold = _threshold;
}

void GaussianBloom::SetBloomIntensity(float _intensity)
{
    cbOptionData_->bloomIntensity = _intensity;
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

void GaussianBloom::CreateRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    /// RootParameterの設定
    RootParameters<3> rootParameters = {};
    rootParameters
        .SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(1, "t1", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(2, "b0", D3D12_SHADER_VISIBILITY_PIXEL);

    descriptionRootSignature.pParameters = rootParameters.GetParams();
    descriptionRootSignature.NumParameters = rootParameters.GetSize();

    /// StaticSamplerの設定
    StaticSamplerDesc staticSamplerDesc = {};
    staticSamplerDesc
        .PresetPointClamp()
        .SetMaxAnisotropy(16)
        .SetShaderRegister(0)
        .SetRegisterSpace(0);

    descriptionRootSignature.pStaticSamplers = &staticSamplerDesc.Get();
    descriptionRootSignature.NumStaticSamplers = 1;

    /// ルートシグネチャのシリアライズ
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

void GaussianBloom::CreatePipelineStateObject()
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
        PipelineStateObject psoBuilder;
        psoBuilder.SetRootSignature(rootSignature_.Get())
            .SetInputLayout(inputLayoutDesc)
            .SetVertexShader(vertexShaderBlob_.Get()->GetBufferPointer(), vertexShaderBlob_.Get()->GetBufferSize())
            .SetPixelShader(pixelShaderBlob_.Get()->GetBufferPointer(), pixelShaderBlob_.Get()->GetBufferSize())
            .SetBlendState(blendDesc.Get())
            .SetRasterizerState(rasterizerDesc)
            .SetRenderTargetFormats(1, &outputTexture_.GetStateTracker().GetFormat(), DXGI_FORMAT_D24_UNORM_S8_UINT)
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
    return;
}

void GaussianBloom::PreDrawSetting(D3D12_GPU_DESCRIPTOR_HANDLE _inputGpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE _outputCpuHandle)
{
    // クリア
    commandList_->ClearRenderTargetView(_outputCpuHandle, &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    // レンダーターゲットを設定 (自分が所有するテクスチャに対して設定)
    commandList_->OMSetRenderTargets(1, &_outputCpuHandle, FALSE, nullptr);

    // PSOとルートシグネチャを設定
    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->SetPipelineState(pso_.Get());

    // 入力テクスチャのSRVを設定する（自分が所有するテクスチャのSRVではないため注意)
    commandList_->SetGraphicsRootDescriptorTable(0, inputGpuHandle_);
    commandList_->SetGraphicsRootDescriptorTable(1, _inputGpuHandle);
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

D3D12_GPU_DESCRIPTOR_HANDLE GaussianBloom::ApplyFilter(D3D12_GPU_DESCRIPTOR_HANDLE _inputGpuHandle, IPostEffect* _pEffect)
{
    _pEffect->SetInputTextureHandle(_inputGpuHandle);
    _pEffect->Setting();
    _pEffect->Apply();
    _pEffect->ToShaderResourceState();
    return _pEffect->GetOutputTextureHandle();
}

void GaussianBloom::CreateResourceCBuffer()
{
    cbOptionResorce_ = DX12Helper::CreateBufferResource(device_, sizeof(GaussianBloomOption));
    cbOptionResorce_->Map(0, nullptr, reinterpret_cast<void**>(&cbOptionData_));

    cbOptionData_->bloomIntensity = 1.0f;
}