#include "CubemapSystem.h"
#include <Core/DirectX12/RootSignature/RootParameters.h>
#include <Core/DirectX12/StaticSamplerDesc/StaticSamplerDesc.h>
#include <Core/DirectX12/BlendDesc.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <config/EngineSetting.h>

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxcapi.h>

void CubemapSystem::Initialize()
{
    device_ = pDx12_->GetDevice();

    /// ルートシグネチャの作成
    _CreateRootSignature();
    /// パイプラインステートオブジェクトの作成
    _CreatePSO();
}

void CubemapSystem::Finalize() {}

void CubemapSystem::_CreateRootSignature()
{
    /// RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    RootParameters rootParam = {};
    rootParam.SetParameter(0, "t0", D3D12_SHADER_VISIBILITY_PIXEL);
    rootParam.SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_VERTEX);
    rootParam.SetParameter(2, "b0", D3D12_SHADER_VISIBILITY_PIXEL);

    descriptionRootSignature.pParameters = rootParam.BuildParams();   // ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = static_cast<UINT>(rootParam.GetSize());   // 配列の長さ

    StaticSamplerDesc staticSampler = {};
    staticSampler
        .PresetPointWrap()
        .SetMaxAnisotropy(16) // 最大異方性を16に設定
        .SetShaderRegister(0) // シェーダーレジスタ番号を0に設定
        .SetRegisterSpace(0); // レジスタスペースを0に設定

    descriptionRootSignature.pStaticSamplers = &staticSampler.Get();
    descriptionRootSignature.NumStaticSamplers = 1;

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    // バイナリをもとに生成
    hr = device_->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void CubemapSystem::_CreatePSO()
{
    IDxcUtils* dxcUtils = pDx12_->GetDxcUtils();
    IDxcCompiler3* dxcCompiler = pDx12_->GetDxcCompiler();
    IDxcIncludeHandler* includeHandler = pDx12_->GetIncludeHandler();

    /// InputLayoutの設定
    D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
    inputElementDesc.SemanticName = "POSITION";
    inputElementDesc.SemanticIndex = 0;
    inputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = &inputElementDesc;
    inputLayoutDesc.NumElements = 1;

    /// BlendStateの設定
    BlendDesc blendDesc = {};
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

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = TRUE;                     // Depthを有効化
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // Depthを書き込まない
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // Depth比較関数をLessEqualに設定

    /// PipelineStateObjectの設定
    try
    {
        PSOBuilder psoBuilder;
        pso_ = psoBuilder.SetRootSignature(rootSignature_.Get())
            .SetInputLayout(inputLayoutDesc)
            .SetVertexShader(vertexShaderBlob_.Get()->GetBufferPointer(), vertexShaderBlob_.Get()->GetBufferSize())
            .SetPixelShader(pixelShaderBlob_.Get()->GetBufferPointer(), pixelShaderBlob_.Get()->GetBufferSize())
            .SetBlendState(blendDesc.Get())
            .SetRasterizerState(rasterizerDesc)
            .SetRenderTargetFormats(1, &NimaEngine::Config::kRenderTargetFormat, DXGI_FORMAT_D24_UNORM_S8_UINT)
            .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
            .SetSampleDesc({ 1, 0 })
            .SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK)
            .SetDepthStencilState(depthStencilDesc)
            .Build(device_);
    }
    catch (const std::exception& _e)
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, _e.what());
        assert(false);
    }
}

void CubemapSystem::DrawSetting(ID3D12GraphicsCommandList* cl)
{
    /// ルートシグネチャをセットする
    cl->SetGraphicsRootSignature(rootSignature_.Get());

    /// グラフィックスパイプラインステートをセットする
    cl->SetPipelineState(pso_.Get());

    /// プリミティブトポロジーをセットする
    cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}