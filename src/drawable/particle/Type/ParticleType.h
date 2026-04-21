#pragma once
#include <Features/TimeMeasurer/DeltaTimeStopWatch.h>
#include <Range.h>
#include <Vector3.h>
#include <Vector4.h>
#include "../Emitter/EmitterData.h"
#include <Math/Transform.h>

using namespace  Type::ParticleEmitter;

enum class ParticleDeleteCondition
{
    LifeTime,
    ZeroAlpha,
};

struct ParticleData
{
    DeltaTimeStopWatch                timer                       = {};
    EulerTransform                  transform                   = {};
    Vector3                         direction                   = {};
    float                           speed_                      = 0.0f;
    Range<Vector3>                  scaleRange                  = {};
    Vector3                         acceleration                = {};
    Vector4                         currentColor                = {};
    Range<Vector4>                  colorRange                  = {};
    float                           seed                        = 0.0f;
    float                           scaleDelayTime              = 0.0f;
    float                           alphaDeltaValue             = 0.0f;
    float                           lifeTime                    = 0.0f;
    float                           currentLifeTime             = 0.0f;
    float                           smoothPower                 = 0.1f;
    Vector3                         accResistance               = {};
    Vector3                         accGravity                  = {};
    Vector3                         velocity                    = {};
    float                           frictionCoef                = {};
    bool                            enableDirectionByVelocity   = false;
    bool                            enableCollisionFloor        = false;
    bool                            enableSmoothRandom          = false;
    float                           radius                      = 0.0f;
    v3::CollisionFloor              collisionFloor              = {};
    ParticleDeleteCondition         deleteCondition             = ParticleDeleteCondition::LifeTime;
};