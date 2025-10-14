#include "LineSystem.h"

#include <DebugTools/Logger/Logger.h>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Win32/WinSystem.h>
#include <config/EngineSetting.h>
#include <Core/DirectX12/Helper/DX12HeapHelper.h>

void LineSystem::Initialize()
{
    ObjectSystemBaseMT::Initialize();

    device_ = pDx12_->GetDevice();
    dxcUtils_ = pDx12_->GetDxcUtils();
    dxcCompiler_ = pDx12_->GetDxcCompiler();
    includeHandler_ = pDx12_->GetIncludeHandler();

    CreateRootSignature();

    SetInputLayout();
    SetBlendDesc();
    SetDSVDesc();

    CreatePipelineState();
}

void LineSystem::PresentDraw()
{
    /// コマンドリストの設定
    DX12Helper::CommandListCommonSetting(pDx12_, commandList_.Get(), rtvHandle_);

    /// ルートシグネチャをセットする
    commandList_->SetGraphicsRootSignature(rootSignature_.Get());
    /// グラフィックスパイプラインステートをセットする
    commandList_->SetPipelineState(pipelineState_.Get());
    /// プリミティブトポロジーをセットする
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
}

void LineSystem::CreateRootSignature()
{
    /// RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // RootParameter作成。複数設定できる
    D3D12_ROOT_PARAMETER rootParameters[2] = {};
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                    // CBVを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                 // PixelShaderで使う
    rootParameters[0].Descriptor.ShaderRegister = 0;                                    // レジスタ番号０とバインド

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                    // CBVを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;                // VertexShaderで使う
    rootParameters[1].Descriptor.ShaderRegister = 0;                                    // レジスタ番号０とバインド


    descriptionRootSignature.pParameters = rootParameters;                              // ルートパラメータ配列へのポインタ
    descriptionRootSignature.NumParameters = _countof(rootParameters);                  // 配列の長さ

    descriptionRootSignature.pStaticSamplers = nullptr;
    descriptionRootSignature.NumStaticSamplers = 0;

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(
            "LineSystem",
            "CreateRootSignature",
            reinterpret_cast<char*>(errorBlob->GetBufferPointer())
        );
        assert(false);
    }
    // バイナリをもとに生成
    hr = device_->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));

    return;
}

void LineSystem::CreatePipelineState()
{
    // RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    // 裏面（時計回り）を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    // 三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.MultisampleEnable = TRUE;        // マルチサンプル有効化
    rasterizerDesc.AntialiasedLineEnable = TRUE;    // ラインのアンチエイリアス有効化

    /// ShaderをCompileする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DX12Helper::CompileShader(
        kVertexShaderPath,
        L"vs_6_0",
        dxcUtils_,
        dxcCompiler_,
        includeHandler_
    );
    assert(vertexShaderBlob != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DX12Helper::CompileShader(
       kPixelShaderPath,
        L"ps_6_0",
        dxcUtils_,
        dxcCompiler_,
        includeHandler_
    );
    assert(pixelShaderBlob != nullptr);

    /// PSOを生成する
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
    pipelineStateDesc.pRootSignature = rootSignature_.Get(); // RootSignature
    pipelineStateDesc.InputLayout = inputLayoutDesc_; // InputLayout
    pipelineStateDesc.VS = { vertexShaderBlob.Get()->GetBufferPointer(), vertexShaderBlob.Get()->GetBufferSize() };
    pipelineStateDesc.PS = { pixelShaderBlob.Get()->GetBufferPointer(), pixelShaderBlob.Get()->GetBufferSize() };
    pipelineStateDesc.BlendState = blendDesc_; // BlendState
    pipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
    // 書き込むRTVの情報
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = NimaEngine::Config::kRenderTargetFormat;
    // 利用するトポロジ（形状）のタイプ。ライン
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    // どのように画面に色を打ち込むかの設定
    pipelineStateDesc.SampleDesc.Count = 1;
    pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    // DepthStencilの設定
    pipelineStateDesc.DepthStencilState = depthStencilDesc_;
    pipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    // 実際に生成
    HRESULT hr = device_->CreateGraphicsPipelineState(&pipelineStateDesc,
        IID_PPV_ARGS(&pipelineState_));

    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(
            "LineSystem",
            __func__,
            "Failed to create pipeline state"
        );
        assert(false);
    }

    return;

}

void LineSystem::SetInputLayout()
{
    /// InputLayout
    inputElementDescs_[0].SemanticName = "POSITION";
    inputElementDescs_[0].SemanticIndex = 0;
    inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
    inputLayoutDesc_.NumElements = _countof(inputElementDescs_);

    return;
}

void LineSystem::SetBlendDesc()
{
    /// BlendStateの設定
    blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc_.RenderTarget[0].BlendEnable = TRUE;
    blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

    blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    return;
}

void LineSystem::SetDSVDesc()
{
    uint32_t clientWidth = WinSystem::clientWidth;
    uint32_t clientHeight = WinSystem::clientHeight;

    // DespStencilResource
    depthStencilResource_ = DX12Helper::CreateDepthStencilTextureResource(device_, clientWidth, clientHeight);
    // DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないため、ShaderVisibleはfalse
    dsvDescriptorHeap_ = DX12HeapHelper::CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

    /// DSVの設定
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;                 // Format。基本的にはResourceに合わせる
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;          // 2dTexture

    device_->CreateDepthStencilView(
        depthStencilResource_.Get(),
        &dsvDesc,
        dsvDescriptorHeap_.Get()->GetCPUDescriptorHandleForHeapStart()
    );


    /// DepthStencilStateの設定
    depthStencilDesc_.DepthEnable = true;
    depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}
