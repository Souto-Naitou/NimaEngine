#pragma once
#include <Vector4.h>
#include <Vector3.h>
#include <Vector2.h>
#include <Matrix4x4.h>
#include <cstdint>

#pragma warning(disable: 4324) // 構造体のメンバがアライメントのためにパディングされる

/// <summary>
/// ライティングの種類
/// </summary>
enum class LightingType : int32_t
{
    LambertianReflectance,
    HarfLambert
};

/// <summary>
/// 平行光源
/// </summary>
struct DirectionalLight
{
    Vector4 color; //!< ライトの色
    Vector3 direction; //!< ライトの向き (正規化必須)
    float intensity; //!< 輝度
};

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

/// <summary>
/// テクスチャのタイル設定
/// </summary>
struct TilingData
{
    Vector2 tilingMultiply;
    int padding[2];
};