#pragma once
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <Math/Transform.h>
#include <Matrix4x4.h>
#include <Features/RandomGenerator/RandomGenerator.h>
#include <string>
#include <memory>

/// <summary>
/// カメラクラス
/// </summary>
class GameEye
{
public:
    GameEye();
    virtual ~GameEye();

    /// <summary>
    /// カメラの行列や揺れ効果などの更新を行います。
    /// </summary>
    virtual void        Update();

    /// <summary>
    /// カメラのデバッグUIを描画します。
    /// </summary>
    virtual void ImGui();

public:
    /// <summary>
    /// 指定範囲でカメラをランダムシェイクさせます。
    /// </summary>
    /// <param name="begin">シェイク範囲の最小値。</param>
    /// <param name="end">シェイク範囲の最大値。</param>
    void Shake(const Vector3& begin, const Vector3& end);

    /// <summary>
    /// 指定強度でカメラをシェイクさせます。
    /// </summary>
    /// <param name="power">シェイク強度。</param>
    void Shake(float power);

public: /// Getter
    const EulerTransform&   GetTransform() const            { return transform_; }
    const Matrix4x4&        GetWorldMatrix() const          { return wMatrix_; }
    const Matrix4x4&        GetViewMatrix() const           { return vMatrix_; }
    const Matrix4x4&        GetProjectionMatrix() const     { return pMatrix_; }
    const Matrix4x4&        GetViewProjectionMatrix() const { return vpMatrix_; }
    const std::string&      GetName() const                 { return name_; }

public: /// Setter
    void SetTransform(const EulerTransform& transform)     { transform_ = transform; }
    void SetRotate(const Vector3& rotate)                  { transform_.rotate = rotate; }
    void SetTranslate(const Vector3& translate)            { transform_.translate = translate; }
    void SetFov(float fov)                                 { fovY_ = fov; }
    void SetAspectRatio(float aspect)                      { aspectRatio_ = aspect; }
    void SetNearClip(float val)                            { nearClip_ = val; }
    void SetFarClip(float val)                             { farClip_ = val; }
    void SetName(const std::string& name)                  { name_ = name; }
    void SetFocalLength(float focalLength);

private: /// メンバ変数
    std::unique_ptr<DebugEntry<GameEye>> pDebugEntry_ = nullptr;

    std::string         name_           = "unnamed";
    EulerTransform      transform_      = {};       // 位置、回転、拡大縮小
    Matrix4x4           wMatrix_        = {};       // ワールド行列
    Matrix4x4           vMatrix_        = {};       // ビュー行列
    Matrix4x4           pMatrix_        = {};       // プロジェクション行列
    Matrix4x4           vpMatrix_       = {};       // ビュープロジェクション行列
    float               fovY_           = 0.0f;
    float               focalLength_    = 0.0f;
    float               sensorHeight_   = 36.0f;     // センサーの高さ
    float               aspectRatio_    = 0.0f;
    float               nearClip_       = 0.0f;
    float               farClip_        = 0.0f;
    Vector3             shakePositon_   = {};

    /// <summary>
    /// 焦点距離から視野角(FOV) を再計算します。
    /// </summary>
    void _UpdateFovFromFocalLength();

protected:



private:
    RandomGenerator* pRandomGenerator_ = nullptr;
};