#include "./GltfModel.h"
#include <DebugTools/DebugManager/DebugManager.h>
#include <Core/DirectX12/TextureManager.h>
#include <Features/Model/Helper/AnimationHelper.h>
#include <stdexcept>
#include <Core/DirectX12/Helper/DX12Helper.h>

GltfModel::~GltfModel()
{
    #ifdef _DEBUG
    if (is_called_finalize_ == false)
    {
        assert(false && "GltfModel::Finalize() must be called before destruction.");
    }
    #endif // _DEBUG
}

void GltfModel::Initialize()
{
    #ifdef _DEBUG
    is_called_finalize_ = false;
    #endif // _DEBUG
    isOverwroteTexture_ = false;
    timer_.Reset();
    timer_.Start();
    srvManager_ = SRVManager::GetInstance();
}

void GltfModel::Finalize()
{
    #ifdef _DEBUG
    is_called_finalize_ = true;
    #endif // _DEBUG
}

void GltfModel::Update()
{
    // 遅延クローン処理
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

    //this->_UpdateLocalMatrixByAnimation();

    this->_ApplyAnimationToSkeleton();

    this->_UpdateSkeleton();

    this->_UpdateSkinCluster();
}

void GltfModel::Draw(ID3D12GraphicsCommandList* cl, uint32_t instanceCount /*= 1*/)
{
    if (isReadyDraw_ == false) return;

    // 頂点バッファビューを設定
    cl->IASetVertexBuffers(0, 1, &vertexBufferView_);
    // インデックスバッファを設定
    cl->IASetIndexBuffer(&indexBufferView_);
    // SRVの設定
    cl->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
    // 描画！（DrawCall/ドローコール）
    cl->DrawIndexedInstanced(
        static_cast<UINT>(modelData_.indices.size()), // インデックス数
        instanceCount, // インスタンス数
        0, // インデックスバッファの開始オフセット
        0, // 頂点バッファの開始オフセット
        0  // グループID（インスタンス化描画用）
    );
}

void GltfModel::ChangeTexture(D3D12_GPU_DESCRIPTOR_HANDLE texSrvHnd)
{
    textureSrvHandleGPU_ = texSrvHnd;
    isOverwroteTexture_ = true;
}

void GltfModel::CreateGPUResource()
{
    /// 頂点リソースを作成
    this->CreateVertexResource();

    /// インデックスリソースを作成
    this->CreateIndexResource();

    /// テクスチャを読み込む
    this->_LoadModelTexture();

    /// スキニング結果を格納するリソースを作成
    this->CreateSkinnedResource();

    /// UAVの生成
    this->CreateUAV();

    // フラグを立てる
    isReadyDraw_ = true;
}

bool GltfModel::IsEndLoading() const
{
    return isReadyDraw_;
}

void GltfModel::Clone(IModel* src)
{
    if (src == nullptr) return;

    isOverwroteTexture_ = false;

    // クローン元がGltfModelでない場合はエラー
    GltfModel* pSrc = dynamic_cast<GltfModel*>(src);
    if (!pSrc)
    {
        throw std::runtime_error("GltfModel::Clone: Source model is not a GltfModel.");
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

std::unique_ptr<IModel> GltfModel::Cloned()
{
    std::unique_ptr<GltfModel> pCloned = std::make_unique<GltfModel>();
    pCloned->isOverwroteTexture_ = false;  
    pCloned->pDx12_ = this->pDx12_;
    // モデルデータをコピー
    pCloned->modelData_ = this->modelData_;
    pCloned->animationData_ = this->animationData_;
    // テクスチャのSRVハンドルをコピー
    //   NOTE: クローン元のテクスチャを上書きしていない場合に限る
    if (!isOverwroteTexture_)
    {
        pCloned->textureSrvHandleGPU_ = this->textureSrvHandleGPU_;
    }
    pCloned->skeleton_ = this->skeleton_;
    pCloned->skinCluster_ = this->skinCluster_;
    // リソースを作成
    pCloned->CreateVertexResource();
    pCloned->CreateIndexResource();
    pCloned->CreateSkinnedResource();
    pCloned->CreateUAV();

    pCloned->isReadyDraw_ = true;

    return pCloned;
}

void GltfModel::DispatchSkinning()
{
    auto cl = pDx12_->GetCommandList();

    // リソースの状態を更新
    resourceSkinned_.GetStateTracker().ChangeState(cl, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    // 準備
    ID3D12DescriptorHeap* ppHeaps[] = { srvManager_->GetDescriptorHeap() };
    cl->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
    cl->SetComputeRootDescriptorTable(0, skinCluster_.srvHandlePalette.second);
    cl->SetComputeRootDescriptorTable(1, srvHandleGpuInputVertex_);
    cl->SetComputeRootDescriptorTable(2, skinCluster_.srvHandleInfluence.second);
    cl->SetComputeRootDescriptorTable(3, srvHandleGpuSkinned_);
    cl->SetComputeRootConstantBufferView(4, skinCluster_.resourceSkinningInformation->GetGPUVirtualAddress());

    cl->Dispatch(
        static_cast<uint32_t>(modelData_.vertices.size() + 1023) / 1024, // グループ数X
        1, // グループ数Y
        1  // グループ数Z
    );

    // リソースの状態を更新
    resourceSkinned_.GetStateTracker().ChangeState(cl, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

D3D12_VERTEX_BUFFER_VIEW GltfModel::GetVertexBufferView() const
{
    return vertexBufferView_;
}

size_t GltfModel::GetVertexCount() const
{
    return modelData_.vertices.size();
}

D3D12_GPU_DESCRIPTOR_HANDLE GltfModel::GetTextureSrvHandle() const
{
    return textureSrvHandleGPU_;
}

ModelData* GltfModel::GetModelData()
{
    return &modelData_;
}

Animation* GltfModel::GetAnimationData()
{
    return &animationData_;
}

Skeleton* GltfModel::GetSkeleton()
{
    return &skeleton_;
}

SkinCluster* GltfModel::GetSkinCluster()
{
    return &skinCluster_;
}

void GltfModel::CreateVertexResource()
{
    /// 頂点リソースを作成
    vertexResource_ = DX12Helper::CreateBufferResource(pDx12_->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

    /// 頂点データを初期化
    std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

    /// SRVの生成
    srvIndexInputVertex_ = srvManager_->Allocate();
    srvManager_->CreateForStructuredBuffer(srvIndexInputVertex_, vertexResource_.Get(), static_cast<uint32_t>(modelData_.vertices.size()), sizeof(VertexData));
    srvHandleGpuInputVertex_ = srvManager_->GetGPUDescriptorHandle(srvIndexInputVertex_);
}

void GltfModel::CreateIndexResource()
{
    /// インデックスリソースを作成
    indexResource_ = DX12Helper::CreateBufferResource(pDx12_->GetDevice(), sizeof(uint32_t) * modelData_.indices.size());
    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

    /// インデックスデータを初期化
    std::memcpy(indexData_, modelData_.indices.data(), sizeof(uint32_t) * modelData_.indices.size());

    /// インデックスバッファービューを初期化
    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = static_cast<uint32_t>(sizeof(uint32_t) * modelData_.indices.size());
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void GltfModel::CreateSkinnedResource()
{
    resourceSkinned_.GetResource() = DX12Helper::CreateBufferResource(
        pDx12_->GetDevice(),
        sizeof(VertexData) * modelData_.vertices.size(),
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    /// 頂点バッファービューを初期化
    vertexBufferView_.BufferLocation = resourceSkinned_.GetResource()->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<uint32_t>(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
}


void GltfModel::_LoadModelTexture()
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

void GltfModel::CopyFrom(GltfModel* pCopySrc)
{
    this->pDx12_ = pCopySrc->pDx12_;
    // モデルデータをコピー
    this->modelData_ = pCopySrc->modelData_;
    this->animationData_ = pCopySrc->animationData_;
    // テクスチャのSRVハンドルをコピー
    //   NOTE: クローン元のテクスチャを上書きしていない場合に限る
    if (!isOverwroteTexture_)
    {
        this->textureSrvHandleGPU_ = pCopySrc->textureSrvHandleGPU_;
    }
    this->skeleton_ = pCopySrc->skeleton_;
    this->skinCluster_ = pCopySrc->skinCluster_;
    // リソースを作成
    this->CreateVertexResource();
    this->CreateIndexResource();
    this->CreateSkinnedResource();
    this->CreateUAV();

    isReadyDraw_ = true;
}

void GltfModel::_UpdateLocalMatrixByAnimation()
{
    if (timer_.GetNow<float>() >= animationData_.duration)
    {
        timer_.Reset();
        timer_.Start();
    }

    // RootAnimationの取得
    NodeAnimation& rootNodeAnim = animationData_.nodeAnimations[modelData_.rootNode.name];
    Vector3 scale = Helper::Animation::CalculateValue(rootNodeAnim.scale, timer_.GetNow<float>());
    Quaternion rotation = Helper::Animation::CalculateValue(rootNodeAnim.rotation, timer_.GetNow<float>());
    Vector3 translation = Helper::Animation::CalculateValue(rootNodeAnim.translate, timer_.GetNow<float>());
    Matrix4x4 localMatrix = Matrix4x4::AffineMatrix(scale, rotation, translation);

    modelData_.rootNode.localMatrix = localMatrix;
}

void GltfModel::_UpdateSkeleton()
{
    for (Joint& joint : skeleton_.GetSkeletonData().joints)
    {
        JointData& jointData = joint.GetJointData();

        jointData.localMatrix = Matrix4x4::AffineMatrix(
            jointData.transform.scale,
            jointData.transform.rotate,
            jointData.transform.translate
        );

        if (jointData.parentIndex)
        {
            // 親ジョイントが存在するなら親の行列を掛ける
            const Matrix4x4& parentSkeletonSpaceMatrix = skeleton_.GetSkeletonData().joints[*jointData.parentIndex].GetJointData().skeletonSpaceMatrix;
            jointData.skeletonSpaceMatrix = jointData.localMatrix * parentSkeletonSpaceMatrix;
        }
        else
        {
            // 親ジョイントがいないのでlocalMatrixとskeletonSpaceMatrixは同じ
            jointData.skeletonSpaceMatrix = jointData.localMatrix;
        }
    }

    skeleton_.Update();
}

void GltfModel::_UpdateSkinCluster()
{
    auto& skeletonData = skeleton_.GetSkeletonData();
    for (size_t jointIndex = 0; jointIndex < skeletonData.joints.size(); ++jointIndex)
    {
        if (jointIndex >= skinCluster_.inverseBindPoseMatrices.size())
        {
            // Jointの数がSkinClusterの逆バインドポーズマトリックスの数を超えた場合はエラー
            throw std::runtime_error("Joint index exceeds the size of inverseBindPoseMatrices in SkinCluster.");
        }
        skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix =
            skinCluster_.inverseBindPoseMatrices[jointIndex] * skeletonData.joints[jointIndex].GetJointData().skeletonSpaceMatrix;
        skinCluster_.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
            skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix.Inverse().Transpose();
    }
}

void GltfModel::_ApplyAnimationToSkeleton()
{
    if (timer_.GetNow<float>() >= animationData_.duration)
    {
        timer_.Reset();
        timer_.Start();
    }

    SkeletonData& skeletonData = skeleton_.GetSkeletonData();
    for (Joint& joint : skeletonData.joints)
    {
        JointData& jointData = joint.GetJointData();
        if (auto it = animationData_.nodeAnimations.find(jointData.name); it != animationData_.nodeAnimations.end())
        {
            const NodeAnimation& rootNodeAnimation = (*it).second;
            // アニメーションの適用
            jointData.transform.translate = Helper::Animation::CalculateValue(rootNodeAnimation.translate, timer_.GetNow<float>());
            jointData.transform.rotate = Helper::Animation::CalculateValue(rootNodeAnimation.rotation, timer_.GetNow<float>());
            jointData.transform.scale = Helper::Animation::CalculateValue(rootNodeAnimation.scale, timer_.GetNow<float>());
        }
    }
}

void GltfModel::CreateUAV()
{
    srvIndexSkinned_ = srvManager_->Allocate();
    srvHandleGpuSkinned_ = srvManager_->GetGPUDescriptorHandle(srvIndexSkinned_);

    srvManager_->CreateUAV4Buffer(
        srvIndexSkinned_,
        resourceSkinned_.GetResource().Get(),
        DXGI_FORMAT_UNKNOWN,
        static_cast<uint32_t>(modelData_.vertices.size()),
        sizeof(VertexData)
    );
}
