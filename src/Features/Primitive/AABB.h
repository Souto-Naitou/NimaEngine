#pragma once

#include <array>

#include <Vector3.h>
#include <Features/Line/Line.h>
#include <Features/Line/LineSystem.h>
#include <Features/GameEye/GameEye.h>

#include <memory>
#include <Color.h>

class AABB
{
public:
    AABB() = default;
    AABB(const Vector3& min, const Vector3& max) : min_(min), max_(max) {}
    ~AABB();

    void Initialize();
    void Update();
    void Draw1F();


public: /// Setter
    void SetMinMax(const Vector3& min, const Vector3& max)
    {
        min_ = min;
        max_ = max;
    }
    void SetGameEye(GameEye** eye) { pGameEye = eye; ModifyGameEye(eye); }
    void SetColor(const RGBA& color) { lines.SetColor(color.to_Vector4()); }


public: /// Getter
    const Vector3& GetMin() const { return min_; }
    const Vector3& GetMax() const { return max_; }


private:
    Vector3 min_; // 最小点
    Vector3 max_; // 最大点
    Line lines = {12}; // 線

    void ModifyGameEye(GameEye** eye);

    LineSystem* pLineSystem = nullptr;
    GameEye** pGameEye = nullptr;
};