#include "Object3dSystem.h"
#include <DebugTools/Logger/Logger.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Window/Window.h>
#include <Core/DirectX12/Helper/DX12HeapHelper.h>
#include <config/EngineSetting.h>
#include <Core/DirectX12/RootSignature/RootSignatureCache.h>
#include <Core/DirectX12/RootSignature/RootSignatureDesc.h>
#include <Core/DirectX12/StaticSamplerDesc/StaticSamplerDesc.h>
#include <cassert>

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxcapi.h>

Object3dSystem::Object3dSystem()
{
}

void Object3dSystem::Initialize()
{
    /// ルートシグネチャの作成
    CreateRootSignature();

    /// シェーダーのコンパイル
    auto dxcUtils = pDx12_->GetDxcUtils();
    auto dxcCompiler = pDx12_->GetDxcCompiler();
    auto includeHandler = pDx12_->GetIncludeHandler();
    ComPtr<IDxcBlob> pBlobVS = DX12Helper::CompileShader(kVertexShaderPath, L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    ComPtr<IDxcBlob> pBlobPS = DX12Helper::CompileShader(kPixelShaderPath, L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);

    /// パイプラインステートの作成
    CreateMainPipelineState(pBlobVS.Get(), pBlobPS.Get());
    CreateDepthPipelineState(pBlobVS.Get());
}

void Object3dSystem::DepthDrawSetting(ID3D12GraphicsCommandList* cl)
{
    /// ルートシグネチャをセットする
    cl->SetGraphicsRootSignature(rootSignature_);

    /// グラフィックスパイプラインステートをセットする
    cl->SetPipelineState(psoEarlyZ_.Get());

    /// プリミティブトポロジーをセットする
    cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dSystem::MainDrawSetting(ID3D12GraphicsCommandList* cl)
{
    /// ルートシグネチャをセットする
    cl->SetGraphicsRootSignature(rootSignature_);

    /// グラフィックスパイプラインステートをセットする
    cl->SetPipelineState(psoMain_.Get());

    /// プリミティブトポロジーをセットする
    cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Object3dSystem::SetEnvironmentTexture(D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    environmentTextureSrvHandleGpu_ = handle;
}

void Object3dSystem::CreateRootSignature()
{
    // RootParameter作成。複数設定できるので配列
    rootParameters_
        .SetParameter(0, "b0", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(1, "b0", D3D12_SHADER_VISIBILITY_VERTEX)
        .SetParameter(2, "t0", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(3, "b1", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(4, "b1", D3D12_SHADER_VISIBILITY_VERTEX)
        .SetParameter(5, "b3", D3D12_SHADER_VISIBILITY_PIXEL)
        .SetParameter(6, "b4", D3D12_SHADER_VISIBILITY_PIXEL)   // Lighting
        .SetParameter(7, "b5", D3D12_SHADER_VISIBILITY_PIXEL)   // PointLight
        .SetParameter(8, "t1", D3D12_SHADER_VISIBILITY_PIXEL);  // EnvironmentTexture

    // StaticSamplerの設定
    StaticSamplerDesc sampler;
    sampler.SetFilter(D3D12_FILTER_MAXIMUM_ANISOTROPIC)
           .SetAddressUVW(D3D12_TEXTURE_ADDRESS_MODE_WRAP)
           .SetMaxAnisotropy(16);

    // RootSignatureDescを作成
    RootSignatureDesc desc;
    desc.params = rootParameters_;
    desc.staticSamplers = sampler;
    desc.flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // RootSignatureCacheに登録
    RootSignatureCache::GetInstance()->Register(kRootSignatureId_, desc);

    // RootSignatureを取得または作成
    rootSignature_ = RootSignatureCache::GetInstance()->GetOrCreate(kRootSignatureId_);
}

void Object3dSystem::CreateMainPipelineState(IDxcBlob* pBlobVS, IDxcBlob* pBlobPS)
{
    ID3D12Device* device = pDx12_->GetDevice();
    uint32_t clientWidth = Window::clientWidth;
    uint32_t clientHeight = Window::clientWidth;

    /// InputLayout
    inputElementDescs_[0].SemanticName = "POSITION";
    inputElementDescs_[0].SemanticIndex = 0;
    inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs_[1].SemanticName = "TEXCOORD";
    inputElementDescs_[1].SemanticIndex = 0;
    inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs_[2].SemanticName = "NORMAL";
    inputElementDescs_[2].SemanticIndex = 0;
    inputElementDescs_[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs_[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
    inputLayoutDesc_.NumElements = _countof(inputElementDescs_);

    /// BlendStateの設定
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    /// RasterizerStateの設定
    rasterizerDesc_.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc_.MultisampleEnable = TRUE;  // アンチエイリアス有効化
    rasterizerDesc_.AntialiasedLineEnable = TRUE;  // ラインのアンチエイリアス有効化

    // DepthStencilResource
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = DX12Helper::CreateDepthStencilTextureResource(device, clientWidth, clientHeight);
    // DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないため、ShaderVisibleはfalse
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = DX12HeapHelper::CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
    // DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にはResourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture
    // DSVHeapの先頭にDSVを作る
    device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart());
    // DepthStencilStateの設定
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    // Depthの機能を有効にする
    depthStencilDesc.DepthEnable = true;
    // 書き込みする
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    // 比較関数はLessEqual。つまり、近ければ描画される
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    /// PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_;    // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc_;    // InputLayout
    graphicsPipelineStateDesc.VS = { pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize() };
    graphicsPipelineStateDesc.BlendState = blendDesc;            // BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc_;    // RasterizerState
    // 書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = NimaEngine::Config::kRenderTargetFormat;
    // 利用するトポロジ（形状）のタイプ。三角形
    graphicsPipelineStateDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // どのように画面に色を打ち込むかの設定（気にしなくて良い）
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

    HRESULT hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&psoMain_));
    if (FAILED(hr)) [[unlikely]]
    {
        Logger::GetInstance()->LogError(
            "Object3dSystem",
            __func__,
            "Failed to create pipeline state"
        );
        assert(false);
    }
}

void Object3dSystem::CreateDepthPipelineState(IDxcBlob* pBlobVS)
{
    ID3D12Device* device = pDx12_->GetDevice();
    uint32_t clientWidth = Window::clientWidth;
    uint32_t clientHeight = Window::clientHeight;

    /// BlendDesc
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;

    // DepthStencilResource
    Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = DX12Helper::CreateDepthStencilTextureResource(device, clientWidth, clientHeight);
    // DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないため、ShaderVisibleはfalse
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = DX12HeapHelper::CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
    // DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にはResourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2dTexture
    // DSVHeapの先頭にDSVを作る
    device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap.Get()->GetCPUDescriptorHandleForHeapStart());

    /// DepthStencilStateの設定 (Early-Zのために変更)
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    /// PSOを直接生成
    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_;
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc_;
    graphicsPipelineStateDesc.VS = { pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize() };
    graphicsPipelineStateDesc.PS = { nullptr, 0 };
    graphicsPipelineStateDesc.BlendState = blendDesc;
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc_;

    // 書き込むRTVの情報
    graphicsPipelineStateDesc.NumRenderTargets = 0;

    // 利用するトポロジ（形状）のタイプ。三角形
    graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // どのように画面に色を打ち込むかの設定（気にしなくて良い）
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    // DepthStencilの設定
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    
    HRESULT hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&psoEarlyZ_));
    if (FAILED(hr)) [[unlikely]]
    {
        Logger::GetInstance()->LogError(
            "Object3dSystem",
            __func__,
            "Failed to create pipeline state"
        );
        assert(false);
    }
}

void Object3dSystem::DrawSingle(ID3D12GraphicsCommandList* cl, Object3dSystem::CommandListData& data)
{
    // モデルがnullptrでない場合のみ描画
    if (!data.model) return;

    // =============================================
    // [DepthDraw Begin]

    // ルートシグネチャをセットする
    cl->SetGraphicsRootSignature(rootSignature_);
    // グラフィックスパイプラインステートをセットする
    cl->SetPipelineState(psoEarlyZ_.Get());
    // プリミティブトポロジーをセットする
    cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // DSVハンドル取得
    auto dsvHandle = pDx12_->GetDSVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
    cl->OMSetRenderTargets(1, &data.rtvHandle, FALSE, &dsvHandle);
    for (auto& cbuffer : data.cbuffers)
    {
        auto& [key, value] = cbuffer;
        cl->SetGraphicsRootConstantBufferView(key, value->GetGPUVirtualAddress());
    }

    // 共通のCBのリソースを設定する
    cl->SetGraphicsRootDescriptorTable(kRootParameterIndexEnvTexture_, environmentTextureSrvHandleGpu_);
    data.model->Draw(cl);

    // [DepthDraw End]
    // =============================================

    // =============================================
    // [MainDraw Begin]

    /// グラフィックスパイプラインステートをセットする
    cl->SetPipelineState(psoMain_.Get());
    data.model->Draw(cl);

    // [MainDraw End]
    // =============================================
}
