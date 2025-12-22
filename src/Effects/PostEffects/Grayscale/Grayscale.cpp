#include "Grayscale.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/DirectX12/BlendDesc.h>
#include <Core/DirectX12/StaticSamplerDesc/StaticSamplerDesc.h>
#include <Core/DirectX12/RootParameters/RootParameters.h>
#include <Core/DirectX12/PipelineStateObject/PipelineStateObject.h>
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
    this->CreateRootSignature();

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
    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->SetPipelineState(pso_.Get());

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

void Grayscale::CreateRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    RootParameters<2> rootParameters = {};
    rootParameters.SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL);
    rootParameters.SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_PIXEL);

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

void Grayscale::CreatePipelineStateObject()
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
    catch (const std::exception& e)
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, e.what());
        assert(false);
    }
    return;
}

void Grayscale::CreateResorceCBuffer()
{
    optionResource_ = DX12Helper::CreateBufferResource(device_, sizeof(Grayscale));
    optionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    pOption_->power = 1.0f;
}
