#include "Object3dInstancedSystem.h"



void Object3dInstancedSystem::Initialize()
{
    RegisterRootSignature();
    RegisterPipelineState();
}

void Object3dInstancedSystem::DrawSingle(ID3D12GraphicsCommandList* cl, CommandListData& data, uint32_t numInstance)
{
    // モデルがnullptrでない場合のみ描画
    if (!data.model) return;

    // ルートシグネチャをセットする
    cl->SetGraphicsRootSignature(pRootSignature_);
    // グラフィックスパイプラインステートをセットする
    cl->SetPipelineState(pPSO_);
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

    cl->SetGraphicsRootDescriptorTable(1, data.instanceDataHandle);

    // 共通のCBのリソースを設定する
    cl->SetGraphicsRootDescriptorTable(kRootParameterIndexEnvTexture_, envTexSrvHandleGpu_);
    data.model->Draw(cl, numInstance);
}

void Object3dInstancedSystem::RegisterRootSignature()
{
    auto rsCache = RootSignatureCache::GetInstance();
    if (!rsCache->IsExist(rootSignatureId_))
    {
        /// RootSignature作成
        RootSignatureDesc rootSignatureDesc = {};
        rootSignatureDesc.params
            .SetParameter(0, "b0", D3D12_SHADER_VISIBILITY_PIXEL)
            .SetParameter(1, "t0", D3D12_SHADER_VISIBILITY_VERTEX)
            .SetParameter(2, "t0", D3D12_SHADER_VISIBILITY_PIXEL)
            .SetParameter(3, "b1", D3D12_SHADER_VISIBILITY_PIXEL)
            .SetParameter(4, "b3", D3D12_SHADER_VISIBILITY_PIXEL)
            .SetParameter(5, "b4", D3D12_SHADER_VISIBILITY_PIXEL)   // Lighting
            .SetParameter(6, "b5", D3D12_SHADER_VISIBILITY_PIXEL)   // PointLight
            .SetParameter(7, "t1", D3D12_SHADER_VISIBILITY_PIXEL);  // EnvironmentTexture
        /// サンプラーの設定
        rootSignatureDesc.staticSamplers
            .PresetPointWrap()
            .SetMaxAnisotropy(16) // 最大異方性を16に設定
            .SetShaderRegister(0) // シェーダーレジスタ番号を0に設定
            .SetRegisterSpace(0); // レジスタスペースを0に設定
        /// ルートシグネチャ記述子を登録
        rsCache->Register(rootSignatureId_, rootSignatureDesc);
    }

    pRootSignature_ = rsCache->GetOrCreate(rootSignatureId_);
}

void Object3dInstancedSystem::RegisterPipelineState()
{
    auto psoCache = PSOCache::GetInstance();
    if (!psoCache->IsExist(psoId_))
    {
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

        PSODesc desc{};
        desc.vs = vertexShaderPath_;
        desc.ps = pixelShaderPath_;
        desc.rootSignatureID = rootSignatureId_;
        desc.blendState.Initialize(BlendDesc::BlendModes::Test);
        desc.inputLayoutDesc.pInputElementDescs = inputElementDescs;
        desc.inputLayoutDesc.NumElements = _countof(inputElementDescs);
        desc.primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        desc.rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        desc.rasterizerDesc.MultisampleEnable = TRUE;  // アンチエイリアス有効化
        desc.rasterizerDesc.AntialiasedLineEnable = TRUE;  // ラインのアンチエイリアス有効化
        psoCache->Register(psoId_, desc);
    }

    pPSO_ = psoCache->GetOrCreate(psoId_);
}
