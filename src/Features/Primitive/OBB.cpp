#include "OBB.h"
#include "OBB.h"

OBB::~OBB()
{
}

void OBB::Initialize()
{
    pLineSystem = LineSystem::GetInstance();
    lines_.Initialize();
    lines_.SetColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
}

void OBB::Update()
{
    Vector3 vertices[8]
    {
        {-size_.x, -size_.y, -size_.z},     // 0
        {-size_.x,  size_.y, -size_.z},     // 1
        { size_.x,  size_.y, -size_.z},     // 2
        { size_.x, -size_.y, -size_.z},     // 3
        {-size_.x, -size_.y,  size_.z},     // 4
        {-size_.x,  size_.y,  size_.z},     // 5
        { size_.x,  size_.y,  size_.z},     // 6
        { size_.x, -size_.y,  size_.z},     // 7
    };

    for (int i = 0; i < 8; i++)
    {
        vertices[i] = FMath::Transform(vertices[i], rotateMatrix_);
        vertices[i] = vertices[i] + center_;
    }

    lines_[0] = vertices[0];
    lines_[1] = vertices[1];

    lines_[2] = vertices[1];
    lines_[3] = vertices[2];

    lines_[4] = vertices[2];
    lines_[5] = vertices[3];

    lines_[6] = vertices[3];
    lines_[7] = vertices[0];

    lines_[8] = vertices[0];
    lines_[9] = vertices[4];

    lines_[10] = vertices[4];
    lines_[11] = vertices[5];

    lines_[12] = vertices[5];
    lines_[13] = vertices[6];

    lines_[14] = vertices[6];
    lines_[15] = vertices[7];

    lines_[16] = vertices[7];
    lines_[17] = vertices[4];

    lines_[18] = vertices[1];
    lines_[19] = vertices[5];

    lines_[20] = vertices[2];
    lines_[21] = vertices[6];

    lines_[22] = vertices[3];
    lines_[23] = vertices[7];

    lines_.Update();
}

void OBB::Draw1F()
{
    lines_.Draw1F();
}

void OBB::ModifyGameEye(GameEye** eye)
{
    lines_.SetGameEye(eye);
}
