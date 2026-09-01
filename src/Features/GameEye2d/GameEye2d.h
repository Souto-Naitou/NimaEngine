#pragma once
#include <Interfaces/IGameEye.h>
#include <Matrix4x4.h>
#include <Vector3.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <vectormatrix/math/Vector2.h>
#include <string>
#include <memory>


/// <summary>
/// 2Dカメラクラス
/// </summary>
class GameEye2d : public IGameEye
{
public:
    GameEye2d();
    virtual ~GameEye2d() = default;

    // ビュー射影行列の更新
    void Update() override;

    // デバッグUIを描画
    void ImGui();

    // ビュー射影行列を取得
    Matrix4x4   GetViewProjectionMatrix() const  override   { return vpMatrix_; };
    Vector3     GetPosition() const override                { return position_; }
    Matrix4x4   GetWorldMatrix() const override             { return wMatrix_; }

    // 名前を設定
    void    SetName(const std::string& name)        { name_ = name; }
    const std::string& GetName() const override     { return name_; }

    // 座標を設定
    void    SetPosition(const Vector2& position)    { position_ = position; }

    // 回転角度を設定
    void    SetRotation(float rotation)             { rotation_ = rotation; }
    float   GetRotation() const                     { return rotation_; }

    // ズーム倍率を設定
    void    SetZoom(float zoom)                     { zoom_ = zoom; }
    float   GetZoom() const                         { return zoom_; }

    // 幅と高さを設定
    void    SetWidth(float width)                   { width_ = width; }
    float   GetWidth() const                        { return width_; }

    // 高さを設定
    void    SetHeight(float height)                 { height_ = height; }
    float   GetHeight() const                       { return height_; }

    // 幅と高さをまとめて設定
    void    SetSize(const Vector2& size)            { width_ = size.x; height_ = size.y; }
    Vector2 GetSize() const                         { return { width_, height_ }; }


private:
    // デバッグ用エントリ
    std::unique_ptr<DebugEntry<GameEye2d>> pDebugEntry_ = nullptr;

    // 再計算
    void Recalculation();

    // 名前 (デバッグ用)
    std::string name_           = "unnamed";

    // カメラの位置
    Vector2     position_       = { 0.0f, 0.0f };

    // カメラの回転角度 (ラジアン)
    float       rotation_       = 0.0f;

    // カメラのズーム倍率
    float       zoom_           = 1.0f;

    // カメラの幅
    float       width_          = 0.0f;

    // カメラの高さ
    float       height_         = 0.0f;

    // ビュー射影行列
    Matrix4x4   vpMatrix_       = Matrix4x4::Identity();

    // ビュー射影行列の逆行列
    Matrix4x4   invVpMatrix_    = Matrix4x4::Identity();

    // ワールド行列
    Matrix4x4   wMatrix_        = Matrix4x4::Identity();
};