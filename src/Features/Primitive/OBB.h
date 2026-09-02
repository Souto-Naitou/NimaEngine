#pragma once

#include <Vector3.h>
#include <Matrix4x4.h>

#include <drawable/line/Line.h>
#include <drawable/line/LineSystem.h>
#include <Features/GameEye/GameEye.h>

#include <array>
#include <memory>

class OBB
{
public:
    OBB() = default;
    ~OBB();

    void Initialize();
    void Update();
    void Draw1F();


public: /// Setter
    void SetCenter(const Vector3& center) { center_ = center; }
    void SetOrientations(const Vector3& orientation1, const Vector3& orientation2, const Vector3& orientation3)
    {
        orientations_[0] = orientation1;
        orientations_[1] = orientation2;
        orientations_[2] = orientation3;
    }
    void SetOrientations(const Matrix4x4& rotateMatrix)
    {
        orientations_[0] = Vector3(rotateMatrix.m[0][0], rotateMatrix.m[1][0], rotateMatrix.m[2][0]);
        orientations_[1] = Vector3(rotateMatrix.m[0][1], rotateMatrix.m[1][1], rotateMatrix.m[2][1]);
        orientations_[2] = Vector3(rotateMatrix.m[0][2], rotateMatrix.m[1][2], rotateMatrix.m[2][2]);

        rotateMatrix_ = rotateMatrix;
    }
    void SetSize(const Vector3& size) { size_ = size; }
    void SetGameEye(IGameEye** eye) { pGameEye = eye; ModifyGameEye(eye); }


public: /// Getter
    const Vector3& GetCenter() const { return center_; }
    const Vector3& GetOrientation(int index) const { return orientations_[index]; }
    const Vector3& GetSize() const { return size_; }
    const Matrix4x4& GetRotateMatrix() const { return rotateMatrix_; }


private:
    Vector3 center_ = {}; // 中心点
    Vector3 orientations_[3] = {}; // 座標軸。正規化・直交必須
    Vector3 size_ = {}; // 座標軸方向の長さの半分。中心から面までの距離

    Matrix4x4 rotateMatrix_ = {};

    Line lines_ = { 12 }; // 線


private:
    void ModifyGameEye(IGameEye** eye);


private:
    LineSystem* pLineSystem = nullptr;
    IGameEye** pGameEye = nullptr;
};