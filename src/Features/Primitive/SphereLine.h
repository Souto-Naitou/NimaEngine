#pragma once
#include <Features/GameEye/GameEye.h>
#include <drawable/line/Line.h>
#include <cstdint>
#include <memory>
#include <Vector3.h>
#include <Color.h>
#include <Interfaces/IGameEye.h>

class SphereLine
{
public:
    void    Initialize();
    void    Update();
    void    Draw1F();

    void    SetTransform(const EulerTransform& transform)   { pLines_->SetTransform(transform); }
    void    SetGameEye(IGameEye** eye)                      { pGameEye_ = eye; ModifyGameEye(eye); }
    void    SetColor(const RGBA& color)                     { pLines_->SetColor(color.to_Vector4()); }
    EulerTransform GetTransform() const                     { return pLines_->GetTransform(); }

private:
    static constexpr uint32_t   kSegments_      = 32;
    std::unique_ptr<Line>       pLines_         = nullptr;
    IGameEye**                  pGameEye_       = nullptr;

    void    GenerateLineVertices();
    void    ModifyGameEye(IGameEye** eye);
};