#include "Object3dInstanced.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include "Object3dInstancedSystem.h"



Object3dInstanced::Object3dInstanced(uint32_t maxInstanceCount /*= 1000u*/) : kMaxInstanceCount_(maxInstanceCount)
{
    instanceData_.reserve(maxInstanceCount);
}

void Object3dInstanced::Initialize()
{
    /// [ 必要なインスタンスを取得 ]
    pSystem_ = Object3dInstancedSystem::GetInstance();
    pDx12_ = pSystem_->GetDirectX12();
    pDevice_ = pDx12_->GetDevice();
    ppGameEye_ = pSystem_->GetGlobalEye();

    /// [ デフォルトの光源データをSystemから取得 ]
    pDirectionalLight_ = pSystem_->GetDirectionalLight();
    if (!pDirectionalLight_)
    {
        LOG_FATAL("DirectionalLight is not set in Object3dSystem. Please set it before initializing Object3d.");
        assert(pDirectionalLight_ && "DirectionalLight is not set in Object3dSystem. Please set it before initializing Object3d.");
    }
    pPointLight_ = pSystem_->GetPointLight();
    if (!pPointLight_)
    {
        LOG_FATAL("PointLight is not set in Object3dSystem. Please set it before initializing Object3d.");
        assert(pPointLight_ && "PointLight is not set in Object3dSystem. Please set it before initializing Object3d.");
    }

    CreateInstanceDataResource();
    CreateCameraForGPUResource();
    CreateLightSettingResource();
    CreateMaterialResource();
}

void Object3dInstanced::Update()
{
    for (uint32_t i = 0; i < instanceData_.size(); ++i)
    {
        auto& data = instanceData_[i];

        auto xMat = Matrix4x4::RotateXMatrix(data.rotate.x);
        auto yMat = Matrix4x4::RotateYMatrix(data.rotate.y);
        auto zMat = Matrix4x4::RotateZMatrix(data.rotate.z);
        auto rotateMat = xMat * yMat * zMat;

        /// ワールド行列を計算
        Matrix4x4 wMat = Matrix4x4::AffineMatrix(data.scale, data.rotate, data.translate);
        Matrix4x4 wvpMat = {};

        /// カメラの行列計算
        auto vpMat = Matrix4x4::Identity();

        if (ppGameEye_ && *ppGameEye_)
        {
            vpMat = (*ppGameEye_)->GetViewProjectionMatrix();
        }
        wvpMat = wMat * vpMat;

        /// インスタンスデータを更新
        auto& instanceDataForGPU = mappedBuffers_.pInstanceData[i];
        instanceDataForGPU.world = wMat;
        instanceDataForGPU.wvp = wvpMat;
        instanceDataForGPU.color = data.color;
    }

    if (pModel_) pModel_->Update();
}

void Object3dInstanced::DrawCall(ID3D12GraphicsCommandList* cl)
{
    Object3dInstancedSystem::CommandListData data;
    data.cbuffers[static_cast<size_t>(CBufferRegister::Material)]           = resourceMaterial_.Get();
    data.instanceDataHandle = srvGpuHandle_;
    // 2: テクスチャ (予約済み)
    data.cbuffers[static_cast<size_t>(CBufferRegister::DirectionalLight)]   = pDirectionalLight_->GetResource();
    data.cbuffers[static_cast<size_t>(CBufferRegister::Camera)]             = resourceCameraForGPU_.Get();
    data.cbuffers[static_cast<size_t>(CBufferRegister::LightSetting)]       = resourceLightSetting_.Get();
    data.cbuffers[static_cast<size_t>(CBufferRegister::PointLight)]         = pPointLight_->GetResource();
    // 7: 環境マップ (予約済み)
    data.rtvHandle = DrawableBase::GetRTVHandleCPU();
    data.model = pModel_;

    pSystem_->DrawSingle(cl, data, static_cast<uint32_t>(this->size()));
}

void Object3dInstanced::CreateInstanceDataResource()
{
    D3D12_RANGE readRange = { 0, 0 }; // CPUは書き込みのみなので、読み取り範囲は0に設定
    void* ppData = nullptr;
    resourceInstanceData_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(Object3dInstanceDataForGPU) * kMaxInstanceCount_);
    resourceInstanceData_->SetName(L"Object3dInstanced::resourceInstanceData_");
    resourceInstanceData_->Map(0, &readRange, &ppData);
    mappedBuffers_.pInstanceData = std::span<Object3dInstanceDataForGPU>(static_cast<Object3dInstanceDataForGPU*>(ppData), kMaxInstanceCount_);

    // インスタンスデータを初期化
    for (auto& instanceData : mappedBuffers_.pInstanceData)
    {
        instanceData.wvp = Matrix4x4::Identity();
        instanceData.world = Matrix4x4::Identity();
        instanceData.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    SRVManager* srvManager = SRVManager::GetInstance();
    srvIndex_ = srvManager->Allocate();
    srvCpuHandle_ = srvManager->GetCPUDescriptorHandle(srvIndex_);
    srvGpuHandle_ = srvManager->GetGPUDescriptorHandle(srvIndex_);

    srvManager->CreateForStructuredBuffer(srvIndex_, resourceInstanceData_.Get(), static_cast<UINT>(kMaxInstanceCount_), sizeof(ParticleForGPU));
}

void Object3dInstanced::CreateCameraForGPUResource()
{
    D3D12_RANGE readRange = { 0, 0 };
    void* ppData = nullptr;
    resourceCameraForGPU_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(CameraForGPU));
    resourceCameraForGPU_->Map(0, &readRange, &ppData);
    mappedBuffers_.pCameraForGPU = static_cast<CameraForGPU*>(ppData);

    // カメラデータを初期化
    mappedBuffers_.pCameraForGPU->worldPosition = Vector3();
}

void Object3dInstanced::CreateLightSettingResource()
{
    void* ppData = nullptr;
    resourceLightSetting_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(LightSetting));
    resourceLightSetting_->Map(0, nullptr, &ppData);
    option_.pLightSettingData = static_cast<LightSetting*>(ppData);

    // ライティング設定を初期化
    option_.pLightSettingData->enableDirectionalLight = 1;
    option_.pLightSettingData->enablePointLight = 1;
}

void Object3dInstanced::CreateMaterialResource()
{
    // マテリアルリソースを作成
    resourceMaterial_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(MaterialForGPU));
    resourceMaterial_->Map(0, nullptr, reinterpret_cast<void**>(&option_.pMaterialData));
    
    // マテリアルデータを初期化
    option_.pMaterialData->uvTransform = Matrix4x4::Identity();
    option_.pMaterialData->shininess = 1.0f;
    option_.pMaterialData->environmentCoefficient = 1.0f; // 環境係数を初期化
}
