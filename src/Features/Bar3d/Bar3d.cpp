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

void Bar3d::Display(float _sec)
{
    timerDisplay_.Reset();
    timerDisplay_.Start();
    isDisplay_ = true;
    displayTime_ = _sec;
}

void Bar3d::SetPosition(const Vector2& _position)
{
    sprite_current_->SetPosition(_position);
    sprite_max_->SetPosition(_position);
}

void Bar3d::SetSize(const Vector2& _size)
{
    sprite_current_->SetSize(_size);
    sprite_max_->SetSize(_size);
}

Vector2 Bar3d::GetHeadUpPositionOnScreen(const Vector3& _targetPos, const Vector2& _hpbarSize, const GameEye& _camera, float _offsetY)
{
    // 3D空間上のHPバー座標
    Vector3 pos3d = _targetPos;
    pos3d.y += _offsetY;

    const auto& vpMatrix    = _camera.GetViewProjectionMatrix();
    auto        vMatrix     = Matrix4x4::ViewportMatrix(0, 0, static_cast<float>(WinSystem::clientWidth), static_cast<float>(WinSystem::clientHeight), 0.0f, 1.0f);
    auto        vpvMatrix   = vpMatrix * vMatrix;
    auto        pos2d       = FMath::Transform(pos3d, vpvMatrix);

    pos2d.x -= _hpbarSize.x * 0.5f;
    pos2d.y -= _hpbarSize.y * 0.5f;

    return Vector2(pos2d.x, pos2d.y);
}


void Bar3d::InitializeSprite(std::unique_ptr<Sprite>& _sprite, const RGBA& _color)
{
    _sprite = std::make_unique<Sprite>();
    _sprite->Initialize("white.png");
    _sprite->SetColor(_color.to_Vector4());
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
