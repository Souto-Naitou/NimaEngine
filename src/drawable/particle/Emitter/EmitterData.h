#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <Vector3.h>
#include <Vector4.h>
#include <Range.h>
#include <cstdint>

struct ParticleData;

namespace Type
{
    namespace ParticleEmitter
    {
        enum class VelocityDistribution
        {
            Box,
            Sphere
        };

        namespace v1
        {
            struct Data;

            void from_json(const nlohmann::json& _j, Data& _data);
            void to_json(nlohmann::json& _j, const Data& _data);
        }

        namespace v2
        {
            struct Common;
            struct RangeData;
            struct PhysicsData;
            struct Flags;
            struct Data;

            void from_json(const nlohmann::json& _j, Data& _data);
            void to_json(nlohmann::json& _j, const Data& _data);
        }

        namespace v3
        {
            struct Common;
            struct RangeData;
            struct Flags;
            struct PhysicsData;
            struct CollisionFloor;
            struct Data;

            void from_json(const nlohmann::json& _j, Data& _data);
            void to_json(nlohmann::json& _j, const Data& _data);
        }
    }
}

struct Type::ParticleEmitter::v1::Data
{
    static constexpr uint32_t version           = 1;                    // バージョン番号
    std::string     name_                       = {};                   // 名前
    Vector3         scaleFixed_                 = {};                   // 固定スケール
    Range<Vector3>  scaleRange_                 = {};                   // スケール範囲
    Range<Vector3>  scaleRandomRange_           = {};                   // スケールランダム範囲

    float           emitInterval_               = {};                   // 発生間隔
    int32_t         emitNum_                    = {};                   // 発生数
    float           emitterLifeTime_            = {};                   // エミッタ寿命
    float           particleLifeTime_           = {};                   // パーティクル寿命
    float           scaleDelayTime_             = {};                   // スケール遅延時間
    Range<Vector3>  positionRange_              = {};                   // 発生位置範囲
    Vector3         emitPositionFixed_          = {};                   // ランダム発生しない場合の発生位置
    Range<Vector4>  colorRange_                 = {};                   // 色変化始点終点
    float           alphaDeltaValue_            = {};                   // 透明度の変化量
    Range<Vector3>  velocityRandomRange_        = {};                   // 速度ランダム範囲
    Vector3         velocityFixed_              = {};                   // 速度固定
    Range<Vector3>  rotationRandomRange_        = {};                   // 回転ランダム範囲
    Vector3         gravity_                    = {};                   // 重力
    Vector3         resistance_                 = {};                   // 抵抗
    bool            enableRandomVelocity_       = {};                   // ランダム速度
    bool            enableRandomEmit_           = {};                   // ランダム発生
    bool            enableRandomRotation_       = {};                   // ランダム回転
    bool            enableRandomScale_          = {};                   // ランダム拡縮
    bool            enableScaleTransition_      = {};                   // スケール遷移
};

struct Type::ParticleEmitter::v2::Common
{
    Common& operator=(const Common& _rv) = default;
    Vector3         scaleFixed                  = {};                   // 固定スケール
    float           emitInterval                = {};                   // 発生間隔
    int32_t         emitNum                     = {};                   // 発生数
    float           emitterLifeTime             = {};                   // エミッタ寿命
    float           particleLifeTime            = {};                   // パーティクル寿命
    float           scaleDelayTime              = {};                   // スケール遅延時間
    Vector3         emitPositionFixed           = {};                   // ランダム発生しない場合の発生位置
    float           alphaDeltaValue             = {};                   // 透明度の変化量
    Vector3         velocityFixed               = {};                   // 速度固定
};

struct Type::ParticleEmitter::v2::Flags
{
    Flags& operator=(const Flags&) = default;
    bool            enableRandomVelocity        = {};                   // ランダム速度
    bool            enableRandomEmit            = {};                   // ランダム発生
    bool            enableRandomRotation        = {};                   // ランダム回転
    bool            enableRandomScale           = {};                   // ランダム拡縮
    bool            enableScaleTransition       = {};                   // スケール遷移
};

struct Type::ParticleEmitter::v2::RangeData
{
    Range<Vector3>  scale                       = {};                   // スケール範囲
    Range<Vector3>  scaleRandom                 = {};                   // スケールランダム範囲
    Range<Vector3>  position                    = {};                   // 発生位置範囲
    Range<Vector4>  color                       = {};                   // 色変化始点終点
    Range<Vector3>  velocityRandom              = {};                   // 速度ランダム範囲
    Range<Vector3>  rotationRandom              = {};                   // 回転ランダム範囲
};

struct Type::ParticleEmitter::v2::PhysicsData
{
    PhysicsData& operator=(const PhysicsData&) = default;
    Vector3         gravity                     = {};                   // 重力
    Vector3         resistance                  = {};                   // 抵抗
};

struct Type::ParticleEmitter::v2::Data
{
    Data() = default;
    Data(const Type::ParticleEmitter::v1::Data& _rv);
    static constexpr uint32_t version           = 2;                    // バージョン番号
    std::string     name                        = {};                   // 名前
    Common          common                      = {};                   // 共通データ
    RangeData       ranges                      = {};                   // ランダム範囲
    PhysicsData     physics                     = {};                   // 物理データ
    Flags           flags                       = {};                   // フラグ
};

struct Type::ParticleEmitter::v3::Common : public v2::Common
{
    float           radius                      = 0.0f;
};

struct Type::ParticleEmitter::v3::RangeData : public v2::RangeData
{
    Range<float>    velocityRandomSphere        = {};                 // SphereVelocityDistribution用のランダム範囲
};

struct Type::ParticleEmitter::v3::PhysicsData : public v2::PhysicsData
{
    float           frictionCoef                = 0.0f;                 // 摩擦
};

struct Type::ParticleEmitter::v3::Flags : v2::Flags
{
    bool            enableCollisionFloor        = {};                   // 衝突床
    VelocityDistribution velocityDistribution   = VelocityDistribution::Box; // 速度分布タイプ
};

struct Type::ParticleEmitter::v3::CollisionFloor
{
    float           elevation                   = 0.0f;
    float           bounce_power                = 0.0f;
};

struct Type::ParticleEmitter::v3::Data
{
    Data() = default;
    Data(const Type::ParticleEmitter::v2::Data& _rv);
    Data(const Type::ParticleEmitter::v1::Data& _rv);
    
    static constexpr uint32_t version           = 3;                    // バージョン番号
    std::string     name                        = {};
    Common          common                      = {};
    RangeData       ranges                      = {};
    PhysicsData     physics                     = {};
    Flags           flags                       = {};
    CollisionFloor  collisionFloor              = {};                   // 衝突床データ
};