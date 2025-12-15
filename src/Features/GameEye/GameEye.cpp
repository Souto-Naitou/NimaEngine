#include "GameEye.h"
#include <Core/Win32/WinSystem.h>
#include <DebugTools/DebugManager/DebugManager.h>

#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG

GameEye::GameEye()
    : transform_({ Vector3(1.0f, 1.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f) })
    , fovY_(0.45f)
    , aspectRatio_(static_cast<float>(WinSystem::clientWidth) / static_cast<float>(WinSystem::clientHeight))
    , nearClip_(0.1f)
    , farClip_(1000.0f)
    , wMatrix_(Matrix4x4::AffineMatrix(transform_.scale, transform_.rotate, transform_.translate))
    , vMatrix_(wMatrix_.Inverse())
    , pMatrix_(Matrix4x4::PerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_))
    , vpMatrix_(vMatrix_* pMatrix_)
{
    std::string* ptr = &name_; ptr;

    RegisterDebugWindowC("GameEye", name_, GameEye::ImGui, false);
    pRandomGenerator_ = RandomGenerator::GetInstance();
}

GameEye::~GameEye()
{
    DebugManager::GetInstance()->DeleteComponent("GameEye", name_);
}

void GameEye::Update()
{
    wMatrix_ = Matrix4x4::AffineMatrix({ 1.0f, 1.0f, 1.0f }, transform_.rotate, transform_.translate + shakePositon_);
    vMatrix_ = wMatrix_.Inverse();
    pMatrix_ = Matrix4x4::PerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);
    vpMatrix_ = vMatrix_ * pMatrix_;

    shakePositon_ = Vector3();
}

void GameEye::_UpdateFovFromFocalLength()
{
    if (focalLength_ > 0.0f && sensorHeight_ > 0.0f)
    {
        float fovRadians = 2.0f * std::atan(sensorHeight_ / (2.0f * focalLength_));
        fovY_ = fovRadians;
    }
    else
    {
        fovY_ = 0.45f; // デフォルト値
    }
}

void GameEye::ImGui()
{
#ifdef _DEBUG

    ImGui::SeparatorText("Transform");
    ImGui::PushID("TRANSFORM");
    ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
    ImGui::DragFloat3("Rotate", &transform_.rotate.x, 0.01f);
    ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
    ImGui::PopID();

    ImGui::SeparatorText("Perspective options");
    ImGui::PushID("PERSPECTIVE");
    ImGui::DragFloat("FovY", &fovY_, 0.01f);
    ImGui::DragFloat("Aspect Ratio", &aspectRatio_, 0.01f);
    ImGui::DragFloat("Near Clip", &nearClip_, 0.01f);
    ImGui::DragFloat("Far Clip", &farClip_, 0.01f);
    ImGui::PopID();

#endif
}

void GameEye::Shake(const Vector3& begin, const Vector3& end)
{
    shakePositon_ = Vector3(
        pRandomGenerator_->Generate(begin.x, end.x),
        pRandomGenerator_->Generate(begin.y, end.y),
        pRandomGenerator_->Generate(begin.z, end.z)
    );
}

void GameEye::Shake(float power)
{
    shakePositon_ = Vector3(
        pRandomGenerator_->Generate(-power, power),
        pRandomGenerator_->Generate(-power, power),
        pRandomGenerator_->Generate(-power, power)
    );
}

void GameEye::SetFocalLength(float focalLength)
{
    focalLength_ = focalLength;
    _UpdateFovFromFocalLength();
}
