#include "./Bar3d.h"
#include <Core/Win32/WinSystem.h>
#include <Matrix4x4.h>
#include <Vector3.h>

void Bar3d::Initialize(const RGBA& colorContext, const RGBA& colorBG)
{
    // Initialize the background sprite
    InitializeSprite(sprite_max_, colorBG);
    InitializeSprite(sprite_current_, colorContext);
}

void Bar3d::Update()
{
    UpdateDisplayFlagByTimer();

    if (isDisplay_ == false) return;

    CurrentValueSpriteUpdate();

    sprite_max_->Update();
    sprite_current_->Update();
}

void Bar3d::Draw2d()
{
    if (isDisplay_ == false) return;


    sprite_max_->Draw1F();
    sprite_current_->Draw1F();
}

void Bar3d::Display(float sec)
{
    timerDisplay_.Reset();
    timerDisplay_.Start();
    isDisplay_ = true;
    displayTime_ = sec;
}

void Bar3d::SetPosition(const Vector2& position)
{
    sprite_current_->SetPosition(position);
    sprite_max_->SetPosition(position);
}

void Bar3d::SetSize(const Vector2& size)
{
    sprite_current_->SetSize(size);
    sprite_max_->SetSize(size);
}

Vector2 Bar3d::GetHeadUpPositionOnScreen(const Vector3& targetPos, const Vector2& hpbarSize, const GameEye& camera, float offsetY)
{
    // 3D空間上のHPバー座標
    Vector3 pos3d = targetPos;
    pos3d.y += offsetY;

    const auto& vpMatrix    = camera.GetViewProjectionMatrix();
    auto        vMatrix     = Matrix4x4::ViewportMatrix(0, 0, static_cast<float>(WinSystem::clientWidth), static_cast<float>(WinSystem::clientHeight), 0.0f, 1.0f);
    auto        vpvMatrix   = vpMatrix * vMatrix;
    auto        pos2d       = FMath::Transform(pos3d, vpvMatrix);

    pos2d.x -= hpbarSize.x * 0.5f;
    pos2d.y -= hpbarSize.y * 0.5f;

    return Vector2(pos2d.x, pos2d.y);
}


void Bar3d::InitializeSprite(std::unique_ptr<Sprite>& sprite, const RGBA& color)
{
    sprite = std::make_unique<Sprite>();
    sprite->Initialize("white.png");
    sprite->SetColor(color.to_Vector4());
}

void Bar3d::CurrentValueSpriteUpdate()
{
    float percentage = valueCurrent_ / valueMax_;
    Vector2 size = sprite_max_->GetSize();
    Vector2 newSize = Vector2(size.x * percentage, size.y);
    sprite_current_->SetSize(newSize);
}

void Bar3d::UpdateDisplayFlagByTimer()
{
    if (isDisplay_ && enableTimer_)
    {
        if (timerDisplay_.GetNow<float>() >= displayTime_)
        {
            isDisplay_ = false;
            timerDisplay_.Stop();
        }
    }
}
