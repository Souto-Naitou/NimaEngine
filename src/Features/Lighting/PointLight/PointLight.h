#pragma once
#include <memory>

#include <Vector4.h>
#include <Vector3.h>
#include <Features/Model/IModel.h>

class Object3d;

struct alignas(16) PointLightForGPU
{
    int enablePointLight    = 0; //!< ポイントライトの有効無効
    float padding0[3]       = {};
    Vector4 color           = {}; //!< ライトの色
    Vector3 position        = {}; //!< ライトの位置
    float intensity         = 0.0f; //!< 輝度
};

/// <summary>
/// ポイントライト
/// </summary>
class PointLight
{
public:
    PointLight() = default;
    ~PointLight() = default;

    void Initialize();
    void Update();
    void Draw();
    void Finalize();

    PointLight& SetIconModel(IModel* pModel);           //!< アイコンのモデルを設定

    bool IsEnable()                 const   { return enable_; }         //!< ポイントライトが有効かどうかを取得
    const Vector3& GetPosition()    const   { return position_; }        //!< ライトの位置を取得
    const Vector4& GetColor()       const   { return color_; }           //!< ライトの色を取得
    float GetIntensity()            const   { return intensity_; }       //!< 輝度を取得

    bool&       IsEnable()                  { return enable_; }         //!< ポイントライトの有効無効を取得
    Vector3&    GetPosition()               { return position_; }        //!< ライトの位置を取得
    Vector4&    GetColor()                  { return color_; }           //!< ライトの色を取得
    float&      GetIntensity()              { return intensity_; }       //!< 輝度を取得

    PointLightForGPU GetDataForGPU() const;                         //!< GPU用のデータを取得

private:
    bool    enable_                     = 0;            //!< ポイントライトの有効無効
    Vector4 color_                      = {};           //!< ライトの色
    Vector3 position_                   = {};           //!< ライトの位置
    float   intensity_                  = 0.0f;         //!< 輝度
    std::unique_ptr<Object3d> pIcon_    = nullptr;
    IModel* pIconModel_                 = nullptr;      //!< アイコンのモデル
};