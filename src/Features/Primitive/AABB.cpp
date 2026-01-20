#include "AABB.h"

AABB::~AABB()
{
}

void AABB::Initialize()
{
    pLineSystem = LineSystem::GetInstance();

    lines.Initialize();
}

void AABB::Update()
{
    Vector3 vertices[8]
    {
        {min_.x, min_.y, min_.z}, // 0
        {min_.x, max_.y, min_.z}, // 1
        {max_.x, max_.y, min_.z}, // 2
        {max_.x, min_.y, min_.z}, // 3
        {min_.x, min_.y, max_.z}, // 4
        {min_.x, max_.y, max_.z}, // 5
        {max_.x, max_.y, max_.z}, // 6
        {max_.x, min_.y, max_.z}, // 7
    };

    lines[0] = vertices[0];
    lines[1] = vertices[1];

    lines[2] = vertices[1];
    lines[3] = vertices[2];

    lines[4] = vertices[2];
    lines[5] = vertices[3];

    lines[6] = vertices[3];
    lines[7] = vertices[0];

    lines[8] = vertices[0];
    lines[9] = vertices[4];

    lines[10] = vertices[4];
    lines[11] = vertices[5];

    lines[12] = vertices[5];
    lines[13] = vertices[6];

    lines[14] = vertices[6];
    lines[15] = vertices[7];

    lines[16] = vertices[7];
    lines[17] = vertices[4];

    lines[18] = vertices[1];
    lines[19] = vertices[5];

    lines[20] = vertices[2];
    lines[21] = vertices[6];

    lines[22] = vertices[3];
    lines[23] = vertices[7];

    lines.Update();
}

void AABB::Draw1F()
{
    lines.Draw1F();
}

void AABB::ModifyGameEye(GameEye** eye)
{
    lines.SetGameEye(eye);
}
