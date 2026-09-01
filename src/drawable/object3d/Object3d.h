#pragma once

#include "Object3dSystem.h"
#include <drawable/base/DrawableBase.h>
#include <drawable/object3d/Material.h>

#include <d3d12.h>
#include <string>
#include <Features/Model/IModel.h>
#include <Interfaces/IGameEye.h>
#include <Features/Lighting/PointLight.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <memory>
#include <common/structs.h>
#include <Features/Lighting/LightingType.h>
#include <Vector4.h>

/// 前方宣言
class   DirectX12;
class   DebugManager;

/// <summary>
/// 3Dオブジェクト
/// </summary>
class Object3d : public DrawableBase
{
public:
    struct Option
    {
        EulerTransform  transform           = {};
        MaterialForGPU* materialData        = nullptr;
        Vector4*        colorData           = nullptr;
        LightSetting*   lightSettingData    = nullptr;
    };

public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(bool enableDebugWindow = true);

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void DrawCall(ID3D12GraphicsCommandList* cl) override;

    /// <summary>
    /// 終了処理
    /// </summary>
    void Finalize() const;

    /// <summary>
    /// デバッグウィンドウ
    /// </summary>
    void ImGui();


public: /// Getter
    const Vector3& GetScale() const                             { return option_.transform.scale; }
    const Vector3& GetRotate() const                            { return option_.transform.rotate; }
    const Vector3& GetTranslate() const                         { return option_.transform.translate; }
    const Matrix4x4& GetRotateMatrix() const                    { return rotateMatrix_; }
    const Option& GetOption() const                             { return option_; }
    Option& GetOption()                                         { return option_; }


public: /// Setter
    void SetScale(const Vector3& scale)                         { option_.transform.scale = scale; }
    void SetRotate(const Vector3& rotate)                       { option_.transform.rotate = rotate; }
    void SetTranslate(const Vector3& translate)                 { option_.transform.translate = translate; }
    void SetGameEye(IGameEye* pGameEye)                         { pGameEye_ = pGameEye; }
    void SetName(const std::string& name)                       { if(pDebugEntry_) pDebugEntry_->SetName(name); }
    void SetDirectionalLight(DirectionalLight* light)           { pDirectionalLight_ = light; }
    void SetPointLight(PointLight* light)                       { pPointLight_ = light; }
    void SetModel(IModel* pModel)                               { pModel_ = pModel; }


private:
    enum class CBufferRegister
    {
        Material                = 0,
        TransformationMatrix    = 1,
        Texture                 = 2,
        DirectionalLight        = 3,
        Color                   = 4,
        Camera                  = 5,
        LightSetting            = 6,
        PointLight              = 7,
        EnvironmentTexture      = 8,
    };

    std::unique_ptr<DebugEntry<Object3d>>           pDebugEntry_                    = nullptr;
    Matrix4x4                                       rotateMatrix_                   = {};

    bool                                            isUpdate_                       = true;
    bool                                            isDraw_                         = true;
    bool                                            isEnableDebugWindow_            = true;

    Microsoft::WRL::ComPtr<ID3D12Resource>          transformationMatrixResource_   = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          colorResource_                  = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          cameraForGPUResource_           = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          lightSettingResource_           = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          materialResource_               = nullptr;

    Option                                          option_                         = {};
    TransformationMatrix*                           transformationMatrixData_       = nullptr;
    CameraForGPU*                                   cameraForGPU_                   = nullptr;

    bool                                            isEnableLighting_               = true;

    std::string                                     modelPath_                      = {};
    IGameEye*                                       pGameEye_                       = nullptr;
    IGameEye**                                      ppSystemGameEye_                = nullptr;
    IModel*                                         pModel_                         = nullptr;


private: /// 非公開メンバ関数
    void CreateTransformationMatrixResource();
    void CreateColorResource();
    void CreateCameraForGPUResource();
    void CreateLightSettingResource();
    void CreateMaterialResource();


private: /// 他クラスが所持するインスタンスへのポインタ
    DirectionalLight*   pDirectionalLight_  = nullptr;
    PointLight*         pPointLight_        = nullptr;
    DirectX12*          pDx12_              = nullptr;
    ID3D12Device*       pDevice_            = nullptr;
    Object3dSystem*     pSystem_            = nullptr;
};