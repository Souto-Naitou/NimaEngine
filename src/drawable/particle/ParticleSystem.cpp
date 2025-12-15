#include "ParticleSystem.h"
#include <Core/DirectX12/DirectX12.h>
#include <DebugTools/Logger/Logger.h>
#include <Core/DirectX12/Helper/DX12Helper.h>

#include <cassert>
#include <Core/Win32/WinSystem.h>
#include <Core/DirectX12/Helper/DX12HeapHelper.h>
#include <config/EngineSetting.h>
#include <Core/DirectX12/BlendDesc.h>

void ParticleSystem::Initialize()
{
    ObjectSystemBaseMT::Initialize();

    CreateRootSignature();

    CreatePipelineState();
}

void ParticleSystem::PresentDraw()
{
    ID3D12GraphicsCommandList* commandList = pDx12_->GetCommandList();

    /// ルートシグネチャをセットする
    commandList->SetGraphicsRootSignature(rootSignature_.Get());

    /// グラフィックスパイプラインステートをセットする
    commandList->SetPipelineState(graphicsPipelineState_.Get());

    /// プリミティブトポロジーをセットする
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

void ParticleSystem::DrawCall()
{
    if(commandListDatas_.empty()) return;

    auto record = [&](ID3D12GraphicsCommandList* commandList)
    {
        /// コマンドリストの設定
        DX12Helper::CommandListCommonSetting(pDx12_, commandList);

        /// ルートシグネチャをセットする
        commandList->SetGraphicsRootSignature(rootSignature_.Get());

        /// グラフィックスパイプラインステートをセットする
        commandList->SetPipelineState(graphicsPipelineState_.Get());

        /// プリミティブトポロジーをセットする
        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // DSVハンドル取得
        auto dsvHandle = pDx12_->GetDSVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

        // 現在のRTVHandle
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCurrent = {};

        for(auto& data : commandListDatas_)
        {
            // RTVハンドルが変わったらセットし直す
            // Note: おなじRTVハンドルが続くようにソートされている前提 (Canvasを使用してソートされるハズ)
            if (rtvHandleCurrent.ptr != data.rtvHandle.ptr && data.rtvHandle.ptr)
            {
                rtvHandleCurrent = data.rtvHandle;
                commandList->OMSetRenderTargets(1, &data.rtvHandle, FALSE, &dsvHandle);
            }

            commandList->IASetVertexBuffers(0, 1, data.pVBV);
            commandList->SetGraphicsRootDescriptorTable(0, data.srvHandle);
            commandList->SetGraphicsRootDescriptorTable(1, data.textureSrvHandle);
            commandList->DrawInstanced(data.vertexCount, data.instanceCount, 0, 0);
        }
    };

    worker_ = std::async(std::launch::async, record, commandList_.Get());

    return;
}

void ParticleSystem::Sync()
{
    if(!worker_.valid()) return;

    worker_.get();
    commandListDatas_.clear();
}

void ParticleSystem::DrawSingle(ID3D12GraphicsCommandList* commandList, CommandListData& data)
{
    /// ルートシグネチャをセットする
    commandList->SetGraphicsRootSignature(rootSignature_.Get());

    /// グラフィックスパイプラインステートをセットする
    commandList->SetPipelineState(graphicsPipelineState_.Get());

    /// プリミティブトポロジーをセットする
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // DSVハンドル取得
    auto dsvHandle = pDx12_->GetDSVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

    commandList->OMSetRenderTargets(1, &data.rtvHandle, FALSE, &dsvHandle);

    commandList->IASetVertexBuffers(0, 1, data.pVBV);
    commandList->SetGraphicsRootDescriptorTable(0, data.srvHandle);
    commandList->SetGraphicsRootDescriptorTable(1, data.textureSrvHandle);
    commandList->DrawInstanced(data.vertexCount, data.instanceCount, 0, 0);
}

void ParticleSystem::CreateRootSignature()
{
    ID3D12Device* device = pDx12_->GetDevice();

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
    descriptorRange[0].NumDescriptors = 1; // 数は1つ
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

    D3D12_DESCRIPTOR_RANGE descRangeTexture = {};
    descRangeTexture.BaseShaderRegister = 0;
    descRangeTexture.NumDescriptors = 1;
    descRangeTexture.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descRangeTexture.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算

    /// RootSignature作成
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    // RootParameter作成。複数設定できるので配列
    D3D12_ROOT_PARAMETER rootParameters[2] = {};
    //rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                    // CBVを使う
    //rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                 // PixelShaderで使う
    //rootParameters[0].Descriptor.ShaderRegister = 0;                                    // レジスタ番号０とバインド
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       // Tableを使う
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;                // VertexShaderで使う
    rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRange;              // レジスタ番号０とバインド
    rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);  // レジスタ番号０とバインド

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;       // Tableを使う
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                 // VertexShaderで使う
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descRangeTexture;            // DescRangeを指定
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;                          // サイズ

    rootSignatureDesc.pParameters = rootParameters;                                     // ルートパラメータ配列へのポインタ
    rootSignatureDesc.NumParameters = _countof(rootParameters);                         // 配列の長さ

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;                         // BilinearFilter
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;                       // 0 ~ 1の範囲外をリピート
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;                     // 比較しない
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;                                       // ありったけのーを使う
    staticSamplers[0].ShaderRegister = 0;                                               // レジスタ番号0を使用する
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
    rootSignatureDesc.pStaticSamplers = staticSamplers;
    rootSignatureDesc.NumStaticSamplers = _countof(staticSamplers);

    // シリアライズしてバイナリにする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(
            "ParticleSystem",
            "CreateRootSignature",
            reinterpret_cast<char*>(errorBlob->GetBufferPointer())
        );

        assert(false);
    }
    // バイナリをもとに生成
    hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));

}

void ParticleSystem::CreatePipelineState()
{
    ID3D12Device* device = pDx12_->GetDevice();
    IDxcUtils* dxcUtils = pDx12_->GetDxcUtils();
    IDxcCompiler3* dxcCompiler = pDx12_->GetDxcCompiler();
    IDxcIncludeHandler* includeHandler = pDx12_->GetIncludeHandler();
    uint32_t clientWidth = WinSystem::clientWidth;
    uint32_t clientHeight = WinSystem::clientHeight;

    /// InputLayout
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    /// BlendStateの設定
    BlendDesc blendDesc{};
    blendDesc.Initialize(BlendDesc::BlendModes::Test);

    // RasterizerStateの設定
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    // 裏面（時計回り）を表示しない
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    // 三角形の中を塗りつぶす
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

    /// ShaderをCompileする
    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = DX12Helper::CompileShader(kVertexShaderPath,
        L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(vertexShaderBlob != nullptr);

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = DX12Helper::CompileShader(kPixelShaderPath,
        L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
    assert(pixelShaderBlob != nullptr);

    // DespStencilResource
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
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();    // RootSignature
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;    // InputLayout
    graphicsPipelineStateDesc.VS = { vertexShaderBlob.Get()->GetBufferPointer(),
    vertexShaderBlob.Get()->GetBufferSize() };                        // VertexShader
    graphicsPipelineStateDesc.PS = { pixelShaderBlob.Get()->GetBufferPointer(),
    pixelShaderBlob.Get()->GetBufferSize() };                            // PixelShader
    graphicsPipelineStateDesc.BlendState = blendDesc.Get();            // BlendState
    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;    // RasterizerState
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
    // 実際に生成
    HRESULT hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
    if (FAILED(hr)) [[unlikely]]
    {
        Logger::GetInstance()->LogError(
            "ParticleSystem",
            __func__,
            "Failed to create pipeline state"
        );
        assert(false);
    }
}
