#include "Object3d.h"

#include <Matrix4x4.h>
#include <Common/structs.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/Helper/DX12Helper.h>

#if defined _DEBUG
#include <imgui.h>
#endif // _DEBUG


void Object3d::Initialize(bool enableDebugWindow)
{
    /// [ 必要なインスタンスを取得 ]
    pSystem_ = Object3dSystem::GetInstance();
    pDx12_ = pSystem_->GetDirectX12();
    pDevice_ = pDx12_->GetDevice();
    ppSystemGameEye_ = pSystem_->GetGlobalEye();

    isEnableDebugWindow_ = enableDebugWindow;

    /// [ システムからデフォルトのライトを取得 ]
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

    /// [ 座標変換行列リソースを作成 ]
    CreateTransformationMatrixResource();

    /// [ カメラのワールド座標リソースを作成 ]
    CreateCameraForGPUResource();

    /// [ ライト設定リソースを作成 ]
    CreateLightSettingResource();

    /// [ マテリアルリソースを作成 ]
    CreateMaterialResource();

    /// [ 色データリソースを作成 ]
    CreateColorResource();
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

    /// カメラのワールド座標を更新
    if (pGameEye_)
    {
        cameraForGPU_->worldPosition = pGameEye_->GetTransform().translate;
    }
    else if (ppSystemGameEye_ && *ppSystemGameEye_)
    {
        cameraForGPU_->worldPosition = (*ppSystemGameEye_)->GetTransform().translate;
    }

    if (pModel_) pModel_->Update();
}

void Object3d::DrawCall(ID3D12GraphicsCommandList* cl)
{
    if (!isDraw_) return;

    Object3dSystem::CommandListData data;
    data.cbuffers[static_cast<uint32_t>(CBufferRegister::Material)] = materialResource_.Get();
    data.cbuffers[static_cast<uint32_t>(CBufferRegister::TransformationMatrix)] = transformationMatrixResource_.Get();
    // 2: テクスチャ
    data.cbuffers[static_cast<uint32_t>(CBufferRegister::DirectionalLight)] = pDirectionalLight_->GetResource();
    data.cbuffers[static_cast<uint32_t>(CBufferRegister::Color)] = colorResource_.Get();
    data.cbuffers[static_cast<uint32_t>(CBufferRegister::Camera)] = cameraForGPUResource_.Get();
    data.cbuffers[static_cast<uint32_t>(CBufferRegister::LightSetting)] = lightSettingResource_.Get();
    data.cbuffers[static_cast<uint32_t>(CBufferRegister::PointLight)] = pPointLight_->GetResource();
    // 8: 環境マップ
    data.rtvHandle = DrawableBase::GetRTVHandleCPU();
    data.model = pModel_;
    
    // マルチスレッド描画は廃止したため
    // コマンドリストデータの蓄積は行わない
    pSystem_->DrawSingle(cl, data);
}

void Object3d::Finalize() const
{
}

void Object3d::CreateTransformationMatrixResource()
{
    /// 座標変換行列リソースを作成
    transformationMatrixResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(TransformationMatrix));
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    /// 座標変換行列データを初期化
    transformationMatrixData_->wvp = Matrix4x4::Identity();
    transformationMatrixData_->world = Matrix4x4::Identity();
}

void Object3d::CreateColorResource()
{
    colorResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(Vector4));
    colorResource_->Map(0, nullptr, reinterpret_cast<void**>(&option_.colorData));

    *option_.colorData = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void Object3d::CreateCameraForGPUResource()
{
    cameraForGPUResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(CameraForGPU));
    cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPU_));
    cameraForGPU_->worldPosition = Vector3();
}

void Object3d::CreateLightSettingResource()
{
    lightSettingResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(LightSetting));
    lightSettingResource_->Map(0, nullptr, reinterpret_cast<void**>(&option_.lightSettingData));
    option_.lightSettingData->enableDirectionalLight = 1;
    option_.lightSettingData->enablePointLight = 0;
    option_.lightSettingData->lightingType = LightingType::HarfLambert;
}

void Object3d::CreateMaterialResource()
{
    /// マテリアルリソースを作成
    materialResource_ = DX12Helper::CreateBufferResource(pDevice_, sizeof(MaterialForGPU));
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&option_.materialData));
    /// マテリアルデータを初期化
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
        ImGui::ColorEdit4("Color", &option_.colorData->x);
        ImGui::DragFloat2("UV Offset", &material->uvTransform.m[3][0], 0.01f);
        ImGui::DragFloat2("UV Tiling", &material->uvTransform.m[0][0], 0.01f);
        ImGui::DragFloat("Shininess", &material->shininess, 0.01f);
        ImGui::SliderFloat("Environment Coefficient", &material->environmentCoefficient, 0.0f, 1.0f);
        ImGui::PopID();
    }


    /// 平行光源
    ImGui::SeparatorText("Directional Light");
    {
        auto& lightSetting = option_.lightSettingData;
        ImGui::PushID("DIRECTIONAL_LIGHT");
        if (ImGui::Checkbox("Enable Lighting", &isEnableLighting_))
        {
            lightSetting->enableDirectionalLight = isEnableLighting_;
        }

        ImGui::SameLine();

        const char* items[] = { "Lambertian Reflectance", "Half Lambert" };
        ImGui::Combo("##Lighting Type", reinterpret_cast<int*>(&option_.lightSettingData->lightingType), items, 2);

        if (pDirectionalLight_)
        {
            auto& data = pDirectionalLight_->GetData();
            ImGui::DragFloat("Shininess", &option_.materialData->shininess, 0.01f);
            ImGui::ColorEdit4("Color", &data.color.x);
            ImGui::DragFloat3("Direction", &data.direction.x, 0.01f);
            ImGui::DragFloat("Intensity", &data.intensity, 0.01f);
        }
        ImGui::PopID();
    }


    /// ポイントライト
    ImGui::SeparatorText("Point Light");
    {
        ImGui::PushID("POINT_LIGHT");
        if (pPointLight_)
        {
            bool enablePointLight = option_.lightSettingData->enablePointLight;
            ImGui::Checkbox("Enable PointLight", &enablePointLight);
            option_.lightSettingData->enablePointLight = enablePointLight ? 1 : 0;
            auto& data = pPointLight_->GetData();
            ImGui::ColorEdit4("Color", &data.color.x);
            ImGui::DragFloat3("Position", &data.position.x, 0.01f);
            ImGui::DragFloat("Intensity", &data.intensity, 0.01f);
        }
        ImGui::PopID();
    }


    /// タイリング
    ImGui::SeparatorText("Tiling");
    {
        ImGui::PushID("TILING");
        ImGui::DragFloat2("Tiling Multiply", &option_.materialData->tilingMultiply.x, 0.01f);
        ImGui::PopID();
    }


#endif // _DEBUG
}
