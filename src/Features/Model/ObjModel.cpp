#include "./ObjModel.h"

#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Features/Model/Helper/ModelHelper.h>
#include <drawable/object3d/Object3dSystem.h>
#include <Core/DirectX12/TextureManager.h>
#include <Features/Model/ModelManager.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <filesystem>

void ObjModel::Initialize()
{
    isOverwroteTexture_ = false;
}

void ObjModel::Update()
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
}

void ObjModel::CreateGPUResource()
{
    /// 頂点リソースを作成
    CreateVertexResource();

    /// テクスチャを読み込む
    LoadModelTexture();

    // フラグを立てる
    isReadyDraw_ = true;
}

void ObjModel::Clone(IModel* src)
{
    if (src == nullptr) return;

    isOverwroteTexture_ = false;

    auto pSrc = dynamic_cast<ObjModel*>(src);
    if (!pSrc)
    {
        throw std::runtime_error("ObjModel::Clone: Source model is not an ObjModel.");
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

std::unique_ptr<IModel> ObjModel::Cloned()
{
    std::unique_ptr<ObjModel> pCloned = std::make_unique<ObjModel>();

    pCloned->pDx12_ = this->pDx12_;
    // モデルデータをコピー
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

void ObjModel::Draw(ID3D12GraphicsCommandList* cl, uint32_t instanceCount /*= 1*/)
{
    if (isReadyDraw_ == false) return;

    // 頂点バッファを設定
    cl->IASetVertexBuffers(0, 1, &vertexBufferView_);
    // SRVの設定
    cl->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
    // 描画！（DrawCall/ドローコール）
    cl->DrawInstanced(static_cast<uint32_t>(modelData_.vertices.size()), instanceCount, 0, 0);
}

void ObjModel::CreateVertexResource()
{
    /// 頂点リソースを作成
    vertexResource_ = DX12Helper::CreateBufferResource(pDx12_->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    /// 頂点データを初期化
    std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

    /// 頂点バッファービューを初期化
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<uint32_t>(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void ObjModel::LoadModelTexture()
{
    if (isOverwroteTexture_)
    {
        // テクスチャを上書きした場合は、マテリアルファイル記述のテクスチャを使用しない
        return;
    }

    TextureManager* textureManager = TextureManager::GetInstance();
    std::string filePath = modelData_.material.textureFilePath;
    textureManager->LoadTexture(filePath);
    textureSrvHandleGPU_ = textureManager->GetSrvHandleGPU(filePath);
}

void ObjModel::CopyFrom(ObjModel* pCopySrc)
{
    this->pDx12_ = pCopySrc->pDx12_;
    // モデルデータをコピー
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

ModelData* ObjModel::GetModelData()
{
    return &modelData_;
}

size_t ObjModel::GetVertexCount() const
{
    return modelData_.vertices.size();
}

D3D12_VERTEX_BUFFER_VIEW ObjModel::GetVertexBufferView() const
{
    return vertexBufferView_;
}

bool ObjModel::IsEndLoading() const
{
    return isReadyDraw_; 
}

D3D12_GPU_DESCRIPTOR_HANDLE ObjModel::GetTextureSrvHandle() const
{
    return textureSrvHandleGPU_; 
}

void ObjModel::ChangeTexture(D3D12_GPU_DESCRIPTOR_HANDLE texSrvHnd)
{
    textureSrvHandleGPU_ = texSrvHnd;
    isOverwroteTexture_ = true;
}
