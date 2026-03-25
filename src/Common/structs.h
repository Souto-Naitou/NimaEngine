#pragma once
#include <Vector4.h>
#include <Vector3.h>
#include <Matrix4x4.h>
#include <cstdint>

#pragma warning(disable: 4324) // 構造体のメンバがアライメントのためにパディングされる

/// <summary>
/// 変形行列群
/// </summary>
struct TransformationMatrix
{
    Matrix4x4 wvp;
    Matrix4x4 world;
};

/// <summary>
/// GPUに送るパーティクル一粒分のデータ
/// </summary>
struct ParticleForGPU
{
    Matrix4x4 wvp;
    Matrix4x4 world;
    Vector4 color;
};

struct alignas(16) CameraForGPU
{
    Vector3 worldPosition;
    float padding0;
};
