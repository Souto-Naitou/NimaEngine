#pragma once

#include <array>

#include <Vector3.h>
#include <drawable/line/Line.h>
#include <drawable/line/LineSystem.h>
#include <Features/GameEye/GameEye.h>

#include <memory>
#include <Color.h>
#include <Interfaces/IGameEye.h>

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
    void SetGameEye(IGameEye** eye) { pGameEye_ = eye; ModifyGameEye(eye); }
    void SetColor(const RGBA& color) { lines.SetColor(color.to_Vector4()); }


public: /// Getter
    const Vector3& GetMin() const { return min_; }
    const Vector3& GetMax() const { return max_; }


private:
    Vector3 min_; // 最小点
    Vector3 max_; // 最大点
    Line lines = {12}; // 線

    void ModifyGameEye(IGameEye** eye);

    LineSystem* pLineSystem = nullptr;
    IGameEye** pGameEye_ = nullptr;
};