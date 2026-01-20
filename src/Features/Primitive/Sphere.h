#pragma once

#include <cstdint>

#include <Vector3.h>
#include <Features/Line/Line.h>
#include <Features/Line/LineSystem.h>
#include <Features/GameEye/GameEye.h>

#include <memory>
#include <Color.h>

class Sphere
{
public:
    Sphere() = default;
    Sphere(Vector3 center, float radius) : center_(center), radius_(radius) {}
    ~Sphere() = default;

    void Initialize();
    void Update();
    void Draw1F();

    void SetRadius(float radius);
    void SetCenter(Vector3 val);
    Vector3 GetCenter() const { return center_; }
    float   GetRadius() const { return radius_; }
    void SetGameEye(GameEye** eye) { pGameEye = eye; ModifyGameEye(eye); }
    void SetColor(const RGBA& color) { pLines_->SetColor(color.to_Vector4()); }

private:
    static constexpr uint32_t kSegments_ = 32;

    Vector3 center_; // 中心点
    float radius_ = 0.0f; // 半径
    std::unique_ptr<Line> pLines_;// 線

    void ModifyGameEye(GameEye** eye);
    void GenerateLineVertices();

    LineSystem* pLineSystem = nullptr;
    GameEye** pGameEye = nullptr;
};