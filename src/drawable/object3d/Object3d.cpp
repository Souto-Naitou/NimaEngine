#include "Object3d.h"

#include <Matrix4x4.h>
#include <Common/structs.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/Helper/DX12Helper.h>

#if defined _DEBUG
#include <imgui.h>
#endif // _DEBUG


void Object3d::Initialize(bool _enableDebugWindow)
{
    /// 必要なインスタンスを取得
    pSystem_ = Object3dSystem::GetInstance();
    pDx12_ = pSystem_->GetDirectX12();
    device_ = pDx12_->GetDevice();
    ppSystemGameEye_ = pSystem_->GetGlobalEye();

    isEnableDebugWindow_ = _enableDebugWindow;

#if defined _DEBUG
    if (isEnableDebugWindow_)
    {
        pDebugEntry_ = std::make_unique<DebugEntry<Object3d>>("Object3d", "unnamed", this, false);
    }
#endif // _DEBUG

    option_.transform =
    {
        .scale      = Vector3(1.0f, 1.0f, 1.0f),
        .rotate     = Vector3(0.0f, 0.0f, 0.0f),
        .translate  = Vector3(0.0f, 0.0f, 0.0f),
    };

    /// 座標変換行列リソースを作成
    CreateTransformationMatrixResource();

    /// 平行光源リソースを作成
    CreateDirectionalLightResource();

    /// テクスチャのタイリングリソースを作成
    CreateTilingResource();

    /// カメラのワールド座標リソースを作成
    CreateCameraForGPUResource();

    /// ライティングリソースを作成
    CreateLightingResource();

    /// ポイントライトリソースを作成
    CreatePointLightResource();

    /// マテリアルリソースを作成
    CreateMaterialResource();
}

void Object3d::Update()
{
    if (!isUpdate_) return;

    /// 回転行列の更新
    {
        auto xMat = Matrix4x4::RotateXMatrix(option_.transform.rotate.x);
        auto yMat = Matrix4x4::RotateYMatrix(option_.transform.rotate.y);
        auto zMat = Matrix4x4::RotateZMatrix(option_.transform.rotate.z);
        rotateMatrix_ = xMat * yMat * zMat;
    }

    Matrix4x4 wMatrix = Matrix4x4::AffineMatrix(option_.transform.scale, option_.transform.rotate, option_.transform.translate);
    Matrix4x4 wvpMatrix = {};

    /// カメラの行列計算
    auto vpMatrix = Matrix4x4::Identity();
    if (pGameEye_)
    {
        vpMatrix = pGameEye_->GetViewProjectionMatrix();
    }
    else if (ppSystemGameEye_ && *ppSystemGameEye_)
    {
        vpMatrix = (*ppSystemGameEye_)->GetViewProjectionMatrix();
    }

    wvpMatrix = wMatrix * vpMatrix;

    /// 座標変換行列データを更新
    transformationMatrixData_->wvp = wvpMatrix;
    transformationMatrixData_->world = wMatrix;


    /// 平行光源の方向を正規化
    if (directionalLight_)
        directionalLight_->direction = directionalLight_->direction.Normalized();


    /// 平行光源データを更新
    if (directionalLight_)
    {
        *directionalLightData_ = *directionalLight_;
    }


    /// ポイントライトデータを更新
    if (pointLight_)
    {
        *pointLightData_ = pointLight_->GetDataForGPU();
    }


    /// カメラのワールド座標を更新
    if (pGameEye_)
    {
        cameraForGPU_->worldPosition = pGameEye_->GetTransform().translate;
    }
    else if (ppSystemGameEye_ && *ppSystemGameEye_)
    {
        cameraForGPU_->worldPosition = (*ppSystemGameEye_)->GetTransform().translate;
    }
}

void Object3d::DrawCall()
{
    if (!isDraw_ || !isDrawCalled_) return;

    isDrawCalled_ = false;

    Object3dSystem::CommandListData data;
    data.cbuffers[0] = materialResource_.Get();
    data.cbuffers[1] = transformationMatrixResource_.Get();
    data.cbuffers[3] = directionalLightResource_.Get();
    data.cbuffers[4] = tilingResource_.Get();
    data.cbuffers[5] = cameraForGPUResource_.Get();
    data.cbuffers[6] = lightingResource_.Get();
    data.cbuffers[7] = pointLightResource_.Get();
    data.rtvHandle = rtvHandle_;
    data.model = pModel_;

    pSystem_->AddCommandListData(data);
}

void Object3d::Finalize() const
{
}


void Object3d::CreateTransformationMatrixResource()
{
    /// 座標変換行列リソースを作成
    transformationMatrixResource_ = DX12Helper::CreateBufferResource(device_, sizeof(TransformationMatrix));
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    /// 座標変換行列データを初期化
    transformationMatrixData_->wvp = Matrix4x4::Identity();
    transformationMatrixData_->world = Matrix4x4::Identity();
}

void Object3d::CreateDirectionalLightResource()
{
    /// 平行光源リソースを作成
    directionalLightResource_ = DX12Helper::CreateBufferResource(device_, sizeof(DirectionalLight));
    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    /// 平行光源データを初期化
    directionalLightData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    directionalLightData_->direction = Vector3(0.0f, -1.0f, 0.0f);
    directionalLightData_->intensity = 1.0f;
}

void Object3d::CreateTilingResource()
{
    /// テクスチャのタイリングリソースを作成
    tilingResource_ = DX12Helper::CreateBufferResource(device_, sizeof(TilingData));
    tilingResource_->Map(0, nullptr, reinterpret_cast<void**>(&option_.tilingData));
    /// タイリングデータを初期化
    option_.tilingData->tilingMultiply = Vector2(1.0f, 1.0f);
}

void Object3d::CreateCameraForGPUResource()
{
    cameraForGPUResource_ = DX12Helper::CreateBufferResource(device_, sizeof(CameraForGPU));
    cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPU_));
    cameraForGPU_->worldPosition = Vector3();
}

void Object3d::CreateLightingResource()
{
    lightingResource_ = DX12Helper::CreateBufferResource(device_, sizeof(Lighting));
    lightingResource_->Map(0, nullptr, reinterpret_cast<void**>(&option_.lightingData));
    option_.lightingData->enableLighting = 1;
    option_.lightingData->lightingType = LightingType::HarfLambert;
}

void Object3d::CreatePointLightResource()
{
    pointLightResource_ = DX12Helper::CreateBufferResource(device_, sizeof(PointLightForGPU));
    pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));
    pointLightData_->enablePointLight = 0;
    pointLightData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    pointLightData_->position = Vector3(0.0f, 0.0f, 0.0f);
    pointLightData_->intensity = 1.0f;
}

void Object3d::CreateMaterialResource()
{
    /// マテリアルリソースを作成
    materialResource_ = DX12Helper::CreateBufferResource(device_, sizeof(Material));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&option_.materialData));
    /// マテリアルデータを初期化
    option_.materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    option_.materialData->uvTransform = Matrix4x4::Identity();
    option_.materialData->shininess = 1.0f;
    option_.materialData->environmentCoefficient = 1.0f; // 環境係数を初期化
}

void Object3d::ImGui()
{
#ifdef _DEBUG

    ImGui::Checkbox("Update", &isUpdate_);
    ImGui::SameLine();
    ImGui::Checkbox("Draw2D", &isDraw_);

    /// 変形
    ImGui::SeparatorText("Transform");
    {
        ImGui::PushID("TRANSFORM");
        auto& tf = option_.transform;
        ImGui::DragFloat3("Scale", &tf.scale.x, 0.01f);
        ImGui::DragFloat3("Rotate", &tf.rotate.x, 0.01f);
        ImGui::DragFloat3("Translate", &tf.translate.x, 0.01f);
        ImGui::PopID();
    }


    /// マテリアル
    ImGui::SeparatorText("Material");
    {
        ImGui::PushID("MATERIAL");
        auto& material = option_.materialData;
        ImGui::ColorEdit4("Color", &material->color.x);
        ImGui::DragFloat2("UV Offset", &material->uvTransform.m[3][0], 0.01f);
        ImGui::DragFloat2("UV Tiling", &material->uvTransform.m[0][0], 0.01f);
        ImGui::DragFloat("Shininess", &material->shininess, 0.01f);
        ImGui::SliderFloat("Environment Coefficient", &material->environmentCoefficient, 0.0f, 1.0f);
        ImGui::PopID();
    }


    /// 平行光源
    ImGui::SeparatorText("Directional Light");
    {
        auto& lightingData = option_.lightingData;
        ImGui::PushID("DIRECTIONAL_LIGHT");
        if (ImGui::Checkbox("Enable Lighting", &isEnableLighting_))
        {
            lightingData->enableLighting = isEnableLighting_;
        }

        ImGui::SameLine();

        const char* items[] = { "Lambertian Reflectance", "Harf Lambert" };
        ImGui::Combo("##Lighting Type", reinterpret_cast<int*>(&lightingData->lightingType), items, 2);

        if (directionalLight_)
        {
            ImGui::DragFloat("Shininess", &option_.materialData->shininess, 0.01f);
            ImGui::ColorEdit4("Color", &directionalLight_->color.x);
            ImGui::DragFloat3("Direction", &directionalLight_->direction.x, 0.01f);
            ImGui::DragFloat("Intensity", &directionalLight_->intensity, 0.01f);
        }
        ImGui::PopID();
    }


    /// ポイントライト
    ImGui::SeparatorText("Point Light");
    {
        ImGui::PushID("POINT_LIGHT");
        if (pointLight_)
        {
            bool enablePointLight = pointLight_->IsEnable();
            if (ImGui::Checkbox("Enable PointLight", &enablePointLight))
            {
                pointLight_->IsEnable() = true;
            }
            ImGui::ColorEdit4("Color", &pointLight_->GetColor().x);
            ImGui::DragFloat3("Position", &pointLight_->GetPosition().x, 0.01f);
            ImGui::DragFloat("Intensity", &pointLight_->GetIntensity(), 0.01f);
        }
        ImGui::PopID();
    }


    /// タイリング
    ImGui::SeparatorText("Tiling");
    {
        ImGui::PushID("TILING");
        ImGui::DragFloat2("Tiling Multiply", &option_.tilingData->tilingMultiply.x, 0.01f);
        ImGui::PopID();
    }


#endif // _DEBUG
}
