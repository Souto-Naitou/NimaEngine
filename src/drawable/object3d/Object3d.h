#pragma once

#include "Object3dSystem.h"
#include <drawable/base/DrawableBase.h>

#include <d3d12.h>
#include <string>
#include <Features/Model/IModel.h>
#include <Features/GameEye/GameEye.h>
#include <Features/Lighting/PointLight/PointLight.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <memory>
#include <common/structs.h>

/// 前方宣言
struct  DirectionalLight;
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
        EulerTransform  transform      = {};
        Material*       materialData   = nullptr;
        TilingData*     tilingData     = nullptr;
        Lighting*       lightingData   = nullptr;
    };

public:
    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(bool _enableDebugWindow = true);

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
    void SetScale(const Vector3& _scale)                        { option_.transform.scale = _scale; }
    void SetRotate(const Vector3& _rotate)                      { option_.transform.rotate = _rotate; }
    void SetTranslate(const Vector3& _translate)                { option_.transform.translate = _translate; }
    void SetGameEye(GameEye* _pGameEye)                         { pGameEye_ = _pGameEye; }
    void SetName(const std::string& name)                       { if(pDebugEntry_) pDebugEntry_->SetName(name); }
    void SetDirectionalLight(DirectionalLight* _light)          { directionalLight_ = _light; }
    void SetPointLight(PointLight* _light)                      { pointLight_ = _light; }
    void SetModel(IModel* _pModel)                              { pModel_ = _pModel; }


private: /// メンバ変数
    std::unique_ptr<DebugEntry<Object3d>>           pDebugEntry_                    = nullptr;
    Matrix4x4                                       rotateMatrix_                   = {};

    bool                                            isUpdate_                       = true;
    bool                                            isDraw_                         = true;
    bool                                            isEnableDebugWindow_            = true;

    Microsoft::WRL::ComPtr<ID3D12Resource>          transformationMatrixResource_   = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          directionalLightResource_       = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          tilingResource_                 = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          cameraForGPUResource_           = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          lightingResource_               = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          pointLightResource_             = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>          materialResource_               = nullptr;

    Option                                          option_                         = {};
    TransformationMatrix*                           transformationMatrixData_       = nullptr;
    DirectionalLight*                               directionalLightData_           = nullptr;
    DirectionalLight*                               directionalLight_               = nullptr;
    PointLightForGPU*                               pointLightData_                 = nullptr;
    PointLight*                                     pointLight_                     = nullptr;
    CameraForGPU*                                   cameraForGPU_                   = nullptr;

    bool                                            isEnableLighting_               = true;

    std::string                                     modelPath_                      = {};
    GameEye*                                        pGameEye_                       = nullptr;
    GameEye**                                       ppSystemGameEye_                = nullptr;
    IModel*                                         pModel_                         = nullptr;


private: /// 非公開メンバ関数
    void CreateTransformationMatrixResource();
    void CreateDirectionalLightResource();
    void CreateTilingResource();
    void CreateCameraForGPUResource();
    void CreateLightingResource();
    void CreatePointLightResource();
    void CreateMaterialResource();


private: /// 他クラスが所持するインスタンスへのポインタ
    DirectX12*      pDx12_          = nullptr;
    ID3D12Device*   device_         = nullptr;
    Object3dSystem* pSystem_        = nullptr;

    DebugManager*   pDebugManager_  = nullptr;
};