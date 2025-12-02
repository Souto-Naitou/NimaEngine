#include "RingModel.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <Core/DirectX12/TextureManager.h>



RingModel::RingModel(const Params& params)
{
    params_ = params;
    pDx12_ = params.pDx12;
}

void RingModel::Initialize()
{
    pDebugEntry_ = std::make_unique<DebugEntry<RingModel>>("Model", "unnnamed_ring", this, false);
    isOverwroteTexture_ = false;
    this->CreateGPUResource();
}

void RingModel::Finalize()
{
}

void RingModel::Update()
{
    if (pCloneSrc_ != nullptr)
    {
        // クローン元のモデルが読み込み完了している場合
        if (pCloneSrc_->IsEndLoading())
        {
            // クローン元からデータをコピー
            this->CopyFrom(pCloneSrc_);
            pCloneSrc_ = nullptr; // クローン元をリセット
        }
    }

    this->UpdateVertexData();
}

void RingModel::Draw(ID3D12GraphicsCommandList* _cl)
{
    if (isReadyDraw_ == false) return;

    // 頂点バッファを設定
    _cl->IASetVertexBuffers(0, 1, &vertexBufferView_);
    // SRVの設定
    _cl->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
    // 描画！（DrawCall/ドローコール）
    _cl->DrawInstanced(static_cast<uint32_t>(modelData_.vertices.size()), 1, 0, 0);

}

void RingModel::ChangeTexture(D3D12_GPU_DESCRIPTOR_HANDLE _texSrvHnd)
{
    textureSrvHandleGPU_ = _texSrvHnd;
    isOverwroteTexture_ = true;
}

void RingModel::ImGui()
{
    #ifdef _DEBUG

    ImGui::DragFloat("Outer Radius", &params_.radiusOuter, 0.01f, 0.0f, 100.0f);
    ImGui::DragFloat("Inner Radius", &params_.radiusInner, 0.01f, 0.0f, 100.0f);
    ImGui::DragFloat("Angle Start (rad)", &params_.radAngleStart, 0.01f, -std::numbers::pi_v<float> *10.0f, std::numbers::pi_v<float> *10.0f);
    ImGui::DragFloat("Angle End (rad)", &params_.radAngleEnd, 0.01f, -std::numbers::pi_v<float> *10.0f, std::numbers::pi_v<float> *10.0f);
    ImGui::DragFloat("Angle Offset (rad)", &params_.radAngleOffset, 0.01f, -std::numbers::pi_v<float> *10.0f, std::numbers::pi_v<float> *10.0f);

    #endif // _DEBUG
}

void RingModel::SetAngleRange(const Range<float>& range)
{
    if (range.start() <= range.end())
    {
        params_.radAngleStart = range.start();
        params_.radAngleEnd = range.end();
    }
    else
    {
        params_.radAngleStart = range.end();
        params_.radAngleEnd = range.start();
    }
}

void RingModel::SetRadii(const Range<float>& range)
{
    if (range.start() <= range.end())
    {
        params_.radiusInner = range.start();
        params_.radiusOuter = range.end();
    }
    else
    {
        params_.radiusInner = range.end();
        params_.radiusOuter = range.start();
    }
}

void RingModel::CreateGPUResource()
{
    this->CreateVertexResource();
    this->LoadTexture();
    isReadyDraw_ = true;
}

bool RingModel::IsEndLoading() const
{
    return isReadyDraw_;
}

void RingModel::Clone(IModel* _src)
{
    if (_src == nullptr) return;

    isOverwroteTexture_ = false;

    auto pSrc = dynamic_cast<RingModel*>(_src);
    if (!pSrc)
    {
        throw std::runtime_error("RingModel::Clone: Source model is not an RingModel.");
        return;
    }

    if (pSrc->IsEndLoading() == false)
    {
        // Updateで読み込みが終わるまで待つ
        // データのコピーはUpdateで行われる
        pCloneSrc_ = pSrc;
        return;
    }

    this->CopyFrom(pSrc);
}

std::unique_ptr<IModel> RingModel::Cloned()
{
    std::unique_ptr<RingModel> pCloned = std::make_unique<RingModel>(this->params_);

    pCloned->pDx12_ = this->pDx12_;
    pCloned->modelData_ = this->modelData_;

    // テクスチャのSRVハンドルをコピー
    if (!isOverwroteTexture_)
    {
        pCloned->textureSrvHandleGPU_ = this->textureSrvHandleGPU_;
    }
    // 頂点リソースを作成
    pCloned->CreateVertexResource();

    pCloned->isReadyDraw_ = true;

    return pCloned;
}

D3D12_VERTEX_BUFFER_VIEW RingModel::GetVertexBufferView() const
{
    return vertexBufferView_;
}

size_t RingModel::GetVertexCount() const
{
    return modelData_.vertices.size();
}

D3D12_GPU_DESCRIPTOR_HANDLE RingModel::GetTextureSrvHandle() const
{
    return textureSrvHandleGPU_;
}

ModelData* RingModel::GetModelData()
{
    return &modelData_;
}

void RingModel::CreateVertexResource()
{
    const uint32_t countVertices = kSegmentCount * 6; // セグメント数×4頂点
    const size_t kBufferSize = sizeof(VertexData) * countVertices;

    /// 頂点リソースを作成
    vertexResource_ = DX12Helper::CreateBufferResource(pDx12_->GetDevice(), kBufferSize);

    VertexData* mappedData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
    
    // spanに変換
    vertexData_ = std::span<VertexData>(mappedData, countVertices);

    /// 頂点バッファービューを初期化
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<uint32_t>(kBufferSize);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void RingModel::LoadTexture()
{
    if (isOverwroteTexture_)
    {
        // テクスチャを上書きした場合は、マテリアルファイル記述のテクスチャを使用しない
        return;
    }

    TextureManager* textureManager = TextureManager::GetInstance();
    const std::string& filePath = params_.textureFilePath;
    textureManager->LoadTexture(filePath);
    textureSrvHandleGPU_ = textureManager->GetSrvHandleGPU(filePath);
}

void RingModel::CopyFrom(RingModel* pCopySrc)
{
    this->params_ = pCopySrc->params_;
    this->pDx12_ = pCopySrc->pDx12_;
    this->modelData_ = pCopySrc->modelData_;

    // テクスチャのSRVハンドルをコピー
    if (!isOverwroteTexture_)
    {
        this->textureSrvHandleGPU_ = pCopySrc->textureSrvHandleGPU_;
    }
    // 頂点リソースを作成
    this->CreateVertexResource();

    isReadyDraw_ = true;
}

void RingModel::UpdateVertexData()
{
    const float radianPerDivide = (params_.radAngleEnd - params_.radAngleStart) / static_cast<float>(kSegmentCount);

    modelData_.vertices.clear();

    for (uint32_t index = 0; index < kSegmentCount; ++index)
    {
        float angle     = static_cast<float>(index) * radianPerDivide + params_.radAngleStart;
        float angleNext = static_cast<float>(index + 1) * radianPerDivide + params_.radAngleStart;

        // オフセット
        angle += params_.radAngleOffset;
        angleNext += params_.radAngleOffset;

        float sin0 = std::sin(angle);
        float cos0 = std::cos(angle);
        float sin1 = std::sin(angleNext);
        float cos1 = std::cos(angleNext);

        float u0 = static_cast<float>(index) / static_cast<float>(kSegmentCount);
        float u1 = static_cast<float>(index + 1) / static_cast<float>(kSegmentCount);

        VertexData outer0 = {};
        outer0.position = Vector4(params_.radiusOuter * cos0, 0.0f, params_.radiusOuter * sin0, 1.0f);
        outer0.texcoord = Vector2(u0, 0.0f);
        outer0.normal = Vector3(0.0f, 1.0f, 0.0f);

        VertexData inner0 = {};
        inner0.position = Vector4(params_.radiusInner * cos0, 0.0f, params_.radiusInner * sin0, 1.0f);
        inner0.texcoord = Vector2(u0, 1.0f);
        inner0.normal = Vector3(0.0f, 1.0f, 0.0f);

        VertexData outer1 = {};
        outer1.position = Vector4(params_.radiusOuter * cos1, 0.0f, params_.radiusOuter * sin1, 1.0f);
        outer1.texcoord = Vector2(u1, 0.0f);
        outer1.normal = Vector3(0.0f, 1.0f, 0.0f);

        VertexData inner1 = {};
        inner1.position = Vector4(params_.radiusInner * cos1, 0.0f, params_.radiusInner * sin1, 1.0f);
        inner1.texcoord = Vector2(u1, 1.0f);
        inner1.normal = Vector3(0.0f, 1.0f, 0.0f);

        // TriangleListで描画するための頂点データの順番に追加
        modelData_.vertices.push_back(outer0);
        modelData_.vertices.push_back(inner0);
        modelData_.vertices.push_back(outer1);

        modelData_.vertices.push_back(outer1);
        modelData_.vertices.push_back(inner0);
        modelData_.vertices.push_back(inner1);
    }

    std::memcpy(vertexData_.data(), modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
}
