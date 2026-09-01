#include "SphereLine.h"
#include <numbers>



void SphereLine::Initialize()
{
    pLines_ = std::make_unique<Line>(3 * kSegments_);
    pLines_->Initialize();
    pLines_->SetColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
    this->GenerateLineVertices();
}

void SphereLine::Update()
{
    pLines_->Update();
}

void SphereLine::Draw1F()
{
    pLines_->Draw1F();
}

void SphereLine::ModifyGameEye(IGameEye** eye)
{
    pLines_->SetGameEye(eye);
}

void SphereLine::GenerateLineVertices()
{
    constexpr float kTwoPi = std::numbers::pi_v<float> *2.0f;
    constexpr uint32_t kSegments = kSegments_;

    auto addCircle = [&](uint32_t baseLine, Vector3 axis1, Vector3 axis2)
    {
        for (uint32_t i = 0; i < kSegments; ++i)
        {
            float t0 = kTwoPi * i / kSegments;
            float t1 = kTwoPi * (i + 1) / kSegments;

            Vector3 p0 =
                axis1 * cosf(t0) +
                axis2 * sinf(t0);

            Vector3 p1 =
                axis1 * cosf(t1) +
                axis2 * sinf(t1);

            size_t index = (baseLine + i) * 2;
            (*pLines_)[index + 0] = p0;
            (*pLines_)[index + 1] = p1;
        }
    };

    addCircle(0 * kSegments, { 1,0,0 }, { 0,1,0 }); // XY
    addCircle(1 * kSegments, { 1,0,0 }, { 0,0,1 }); // XZ
    addCircle(2 * kSegments, { 0,1,0 }, { 0,0,1 }); // YZ
}