#include "RandomFilter.h"
#include <cassert>
#include <Core/DirectX12/DirectX12.h>
#include <Effects/PostEffects/.Helper/PostEffectHelper.h>
#include <Core/DirectX12/SRVManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <imgui.h>
#include <Core/DirectX12/BlendDesc.h>
#include <Core/DirectX12/StaticSamplerDesc/StaticSamplerDesc.h>
#include <Core/DirectX12/RootParameters/RootParameters.h>

void RandomFilter::Initialize(const PostEffectInitParams& desc)
{
    pDx12_ = desc.pDx12;
    commandList_ = desc.pCommandList;
    device_ = pDx12_->GetDevice();

    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_RandomFilter");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();

    // ルートシグネチャの生成
    this->CreateRootSignature();

    // パイプラインステートの生成
    this->CreatePipelineStateObject();

    // 設定用リソースの生成と初期化
    this->CreateResourceCBuffer();
}

void RandomFilter::Enable(bool _flag)
{
    isEnabled_ = _flag;
}

void RandomFilter::SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle)
{
    inputGpuHandle_ = _gpuHandle;
}

void RandomFilter::SetOpacity(float _opacity)
{
    if (pOption_ != nullptr)
    {
        pOption_->opacity = _opacity;
    }
}

void RandomFilter::SetSeed(float _seed)
{
    if (pOption_ != nullptr)
    {
        pOption_->seed = _seed;
    }
}

bool RandomFilter::Enabled() const
{
    return isEnabled_;
}

D3D12_GPU_DESCRIPTOR_HANDLE RandomFilter::GetOutputTextureHandle() const
{
    return renderTexture_.GetSRVHandleGPU();
}

const std::string& RandomFilter::GetName() const
{
    return name_;
}

RandomFilter::RandomFilterOption& RandomFilter::GetOption()
{
    return *pOption_;
}

const RandomFilter::RandomFilterOption& RandomFilter::GetOption() const
{
    return *pOption_;
}

void RandomFilter::Apply()
{
    commandList_->DrawInstanced(3, 1, 0, 0); // 三角形を1つ描画
}

void RandomFilter::Finalize()
{
}

void RandomFilter::Setting()
{
    // レンダーテクスチャをレンダーターゲット状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // レンダーターゲットを設定 (自分が所有するテクスチャに対して設定)
    commandList_->OMSetRenderTargets(1, &renderTexture_.GetRTVHandle(), FALSE, nullptr);

    // PSOとルートシグネチャを設定
    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    commandList_->SetPipelineState(pso_.GetPSO());

    // 入力テクスチャのSRVを設定する（自分が所有するテクスチャのSRVではないため注意)
    commandList_->SetGraphicsRootDescriptorTable(0, inputGpuHandle_);

    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    commandList_->SetGraphicsRootConstantBufferView(1, optionResource_->GetGPUVirtualAddress());
}

void RandomFilter::OnResizeBefore()
{
    SRVManager::GetInstance()->Deallocate(renderTexture_.GetSRVIndex());
    renderTexture_.GetResource().Reset();
    renderTexture_.GetStateTracker().Reset();
}

void RandomFilter::OnResizeAfter()
{
    // レンダーテクスチャの生成
    Helper::CreateRenderTexture(pDx12_, device_, renderTexture_, "RT_RandomFilter");

    // レンダーテクスチャのSRVを生成
    renderTexture_.CreateSRV();
}

void RandomFilter::ToShaderResourceState()
{
    // レンダーテクスチャをシェーダーリソース状態に変更
    renderTexture_.GetStateTracker().ChangeState(commandList_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void RandomFilter::DebugOverlay()
{
    #ifdef _DEBUG

    ImGui::DragFloat("Seed", &pOption_->seed, 0.01f, FLT_MIN);
    ImGui::DragFloat("Opacity", &pOption_->opacity, 0.01f, FLT_MIN);

    #endif //_DEBUG
}

void RandomFilter::CreateRootSignature()
{
    /// RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // RootParameter作成。複数設定できるので配列
    RootParameters<2> rootParameters = {};
    try
    {
        rootParameters
            .SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL)
            .SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_PIXEL);
    }
    catch (const std::exception& _e)
    {
        Logger::GetInstance()->LogError(
            "RandomFilter",
            __func__,
            _e.what()
        );
    }

    descriptionRootSignature.pParameters = rootParameters.GetParams();                  // ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = rootParameters.GetSize();                  // 配列の長さ

    StaticSamplerDesc staticSampler = {};
    staticSampler
        .PresetPointWrap()                                      // Point&Wrapの設定
        .SetMaxAnisotropy(16)                                   // 最大異方性
        .SetShaderRegister(0)                                   // サンプラーのレジスタ番号
        .SetRegisterSpace(0)                                    // レジスタスペース
        .SetShaderVisibility(D3D12_SHADER_VISIBILITY_PIXEL);    // ピクセルシェーダーで使用

    descriptionRootSignature.pStaticSamplers = &staticSampler.Get();
    descriptionRootSignature.NumStaticSamplers = 1;

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(
            "RandomFilter",
            __func__,
            reinterpret_cast<char*>(errorBlob->GetBufferPointer())
        );

        assert(false);
    }
    // バイナリをもとに生成
    hr = device_->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void RandomFilter::CreatePipelineStateObject()
{
    IDxcUtils* dxcUtils = pDx12_->GetDxcUtils();
    IDxcCompiler3* dxcCompiler = pDx12_->GetDxcCompiler();
    IDxcIncludeHandler* includeHandler = pDx12_->GetIncludeHandler();

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = nullptr;
    inputLayoutDesc.NumElements = 0;

    /// BlendStateの設定
    BlendDesc blendDesc{};
    blendDesc.Initialize(BlendDesc::BlendModes::Alpha);

    /// RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.MultisampleEnable = TRUE;  // アンチエイリアス有効化
    rasterizerDesc.AntialiasedLineEnable = TRUE;  // ラインのアンチエイリアス有効化

    /// ShaderをCompileする
    vertexShaderBlob_ = DX12Helper::CompileShader(kVertexShaderPath, L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vertexShaderBlob_ != nullptr);

    pixelShaderBlob_ = DX12Helper::CompileShader(kPixelShaderPath, L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(pixelShaderBlob_ != nullptr);

    try
    {
        pso_.SetRootSignature(rootSignature_.Get())
            .SetInputLayout(inputLayoutDesc)
            .SetVertexShader(vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize())
            .SetPixelShader(pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize())
            .SetBlendState(blendDesc.Get())
            .SetRasterizerState(rasterizerDesc)
            .SetRenderTargetFormats(1, &renderTexture_.GetStateTracker().GetFormat(), DXGI_FORMAT_D24_UNORM_S8_UINT)
            .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
            .SetSampleDesc({ 1, 0 }) // マルチサンプルなし
            .SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK)
            .Build(device_);
    }
    catch (const std::exception& _e)
    {
        Logger::GetInstance()->LogError(
            "RandomFilter",
            __func__,
            _e.what()
        );
    }

    return;
}

void RandomFilter::CreateResourceCBuffer()
{
    optionResource_ = DX12Helper::CreateBufferResource(device_, sizeof(RandomFilterOption));
    optionResource_->Map(0, nullptr, reinterpret_cast<void**>(&pOption_));

    pOption_->seed = 0.0f;
    pOption_->opacity = 1.0f;
}
