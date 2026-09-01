#include "GameEye2d.h"
#include <Matrix4x4.h>
#include <Core/Window/Window.h>


GameEye2d::GameEye2d()
{
    pDebugEntry_ = std::make_unique<DebugEntry<GameEye2d>>("GameEye2d", name_, this);
    width_ = static_cast<float>(Window::clientWidth);
    height_ = static_cast<float>(Window::clientHeight);
}

void GameEye2d::Update()
{
    this->Recalculation();
}

void GameEye2d::ImGui()
{
    #ifdef _DEBUG

    ImGui::SeparatorText("Transform");
    ImGui::PushID("TRANSFORM");
    Vector2 size = { width_, height_ };
    if (ImGui::DragFloat2("Scale", &size.x, 0.01f))
    {
        width_ = size.x;
        height_ = size.y;
    }
    ImGui::SliderAngle("Rotate", &rotation_);
    ImGui::DragFloat2("Translate", &position_.x, 0.01f);
    ImGui::PopID();

    ImGui::SeparatorText("Perspective options");
    ImGui::PushID("PERSPECTIVE");
    ImGui::DragFloat("Zoom", &zoom_, 0.01f);
    ImGui::PopID();

    #endif
}

void GameEye2d::Recalculation()
{
    const float halfWidth = width_ * 0.5f / zoom_;
    const float halfHeight = height_ * 0.5f / zoom_;

    // 正射影行列の計算
    Matrix4x4 projection = Matrix4x4::OrthographicMatrix(
        -halfWidth, -halfHeight, halfWidth, halfHeight, -1.0f, 1.0f
    );

    // ビュー行列の計算
    Matrix4x4 view = Matrix4x4::TranslateMatrix(Vector2(-position_.x, position_.y)) * Matrix4x4::RotateZMatrix(-rotation_);

    // ビュー射影行列の計算
    vpMatrix_ = view * projection;
    invVpMatrix_ = vpMatrix_.Inverse();

    // ワールド行列の計算
    wMatrix_ = Matrix4x4::RotateZMatrix(-rotation_);
}
