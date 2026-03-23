#pragma once
#include <Matrix4x4.h>
#include <Vector4.h>
#include <Vector2.h>

struct alignas(16) MaterialForGPU
{
    Matrix4x4 uvTransform;
    float shininess;
    float environmentCoefficient; // Environment coefficient for lighting
    Vector2 tilingMultiply;
};