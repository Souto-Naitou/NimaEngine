#include "EmitterData.h"

#include <Utility/JSON/jsonutl.h>
#include <Utility/JSONConvTypeFuncs/JSONConvTypeFuncs.h>

template <typename T>
static void CheckVersion(const nlohmann::json& j,  const T& data)
{
    uint32_t v = 0;
    utl::json::try_assign(j, "version", v);
    if (v != data.version)
    {
        throw std::runtime_error("Invalid version for ParticleEmitter::v1::Data: " + std::to_string(v));
    }
}

namespace Type::ParticleEmitter::v1
{
    void from_json(const nlohmann::json& j, Data& data)
    {
        CheckVersion(j, data);

        utl::json::try_assign(j, "name", data.name_);
        utl::json::try_assign(j, "scaleFixed", data.scaleFixed_);
        utl::json::try_assign(j, "scaleRange", data.scaleRange_);
        utl::json::try_assign(j, "scaleRandomRange", data.scaleRandomRange_);
        utl::json::try_assign(j, "emitInterval", data.emitInterval_);
        utl::json::try_assign(j, "emitNum", data.emitNum_);
        utl::json::try_assign(j, "emitterLifeTime", data.emitterLifeTime_);
        utl::json::try_assign(j, "particleLifeTime", data.particleLifeTime_);
        utl::json::try_assign(j, "scaleDelayTime", data.scaleDelayTime_);
        utl::json::try_assign(j, "positionRange", data.positionRange_);
        utl::json::try_assign(j, "emitPositionFixed", data.emitPositionFixed_);
        utl::json::try_assign(j, "colorRange", data.colorRange_);
        utl::json::try_assign(j, "alphaDeltaValue", data.alphaDeltaValue_);
        utl::json::try_assign(j, "velocityRandomRange", data.velocityRandomRange_);
        utl::json::try_assign(j, "velocityFixed", data.velocityFixed_);
        utl::json::try_assign(j, "rotationRandomRange", data.rotationRandomRange_);
        utl::json::try_assign(j, "gravity", data.gravity_);
        utl::json::try_assign(j, "resistance", data.resistance_);
        utl::json::try_assign(j, "enableRandomVelocity", data.enableRandomVelocity_);
        utl::json::try_assign(j, "enableRandomEmit", data.enableRandomEmit_);
        utl::json::try_assign(j, "enableRandomRotation", data.enableRandomRotation_);
        utl::json::try_assign(j, "enableRandomScale", data.enableRandomScale_);
        utl::json::try_assign(j, "enableScaleTransition", data.enableScaleTransition_);
    }

    void to_json(nlohmann::json& j, const Data& data)
    {
        j["version"]                       = data.version;
        j["name"]                          = data.name_;
        j["scaleFixed"]                    = data.scaleFixed_;
        j["scaleRange"]                    = data.scaleRange_;
        j["scaleRandomRange"]              = data.scaleRandomRange_;
        j["emitInterval"]                  = data.emitInterval_;
        j["emitNum"]                       = data.emitNum_;
        j["emitterLifeTime"]               = data.emitterLifeTime_;
        j["particleLifeTime"]              = data.particleLifeTime_;
        j["scaleDelayTime"]                = data.scaleDelayTime_;
        j["positionRange"]                 = data.positionRange_;
        j["emitPositionFixed"]             = data.emitPositionFixed_;
        j["colorRange"]                    = data.colorRange_;
        j["alphaDeltaValue"]               = data.alphaDeltaValue_;
        j["velocityRandomRange"]           = data.velocityRandomRange_;
        j["velocityFixed"]                 = data.velocityFixed_;
        j["rotationRandomRange"]           = data.rotationRandomRange_;
        j["gravity"]                       = data.gravity_;
        j["resistance"]                    = data.resistance_;
        j["enableRandomVelocity"]          = data.enableRandomVelocity_;
        j["enableRandomEmit"]              = data.enableRandomEmit_;
        j["enableRandomRotation"]          = data.enableRandomRotation_;
        j["enableRandomScale"]             = data.enableRandomScale_;
        j["enableScaleTransition"]         = data.enableScaleTransition_;
    }
}

namespace Type::ParticleEmitter::v2
{
    Data::Data(const Type::ParticleEmitter::v1::Data& rv)
    {
        name = rv.name_;
        common.scaleFixed = rv.scaleFixed_;
        common.emitInterval = rv.emitInterval_;
        common.emitNum = rv.emitNum_;
        common.emitterLifeTime = rv.emitterLifeTime_;
        common.particleLifeTime = rv.particleLifeTime_;
        common.scaleDelayTime = rv.scaleDelayTime_;
        common.emitPositionFixed = rv.emitPositionFixed_;
        common.alphaDeltaValue = rv.alphaDeltaValue_;
        common.velocityFixed = rv.velocityFixed_;
        ranges.scale = rv.scaleRange_;
        ranges.scaleRandom = rv.scaleRandomRange_;
        ranges.position = rv.positionRange_;
        ranges.color = rv.colorRange_;
        ranges.velocityRandom = rv.velocityRandomRange_;
        ranges.rotationRandom = rv.rotationRandomRange_;
        physics.gravity = rv.gravity_;
        physics.resistance = rv.resistance_;
        flags.enableRandomVelocity = rv.enableRandomVelocity_;
        flags.enableRandomEmit = rv.enableRandomEmit_;
        flags.enableRandomRotation = rv.enableRandomRotation_;
        flags.enableRandomScale = rv.enableRandomScale_;
        flags.enableScaleTransition = rv.enableScaleTransition_;
    }

    void from_json(const nlohmann::json& j, Data& data)
    {
        CheckVersion(j, data);

        utl::json::try_assign(j, "name", data.name);

        nlohmann::json j_range;  utl::json::try_assign(j, "ranges", j_range);
        nlohmann::json j_common;  utl::json::try_assign(j, "common", j_common);
        nlohmann::json j_phys;  utl::json::try_assign(j, "physics", j_phys);
        nlohmann::json j_flag;  utl::json::try_assign(j, "flags", j_flag);

        auto& d_common = data.common;

        utl::json::try_assign(j_common, "scaleFixed", d_common.scaleFixed);
        utl::json::try_assign(j_common, "emitInterval", d_common.emitInterval);
        utl::json::try_assign(j_common, "emitNum", d_common.emitNum);
        utl::json::try_assign(j_common, "emitterLifeTime", d_common.emitterLifeTime);
        utl::json::try_assign(j_common, "particleLifeTime", d_common.particleLifeTime);
        utl::json::try_assign(j_common, "scaleDelayTime", d_common.scaleDelayTime);
        utl::json::try_assign(j_common, "emitPositionFixed", d_common.emitPositionFixed);
        utl::json::try_assign(j_common, "alphaDeltaValue", d_common.alphaDeltaValue);
        utl::json::try_assign(j_common, "velocityFixed", d_common.velocityFixed);

        auto& d_range = data.ranges;

        utl::json::try_assign(j_range, "scale", d_range.scale);
        utl::json::try_assign(j_range, "scaleRandom", d_range.scaleRandom);
        utl::json::try_assign(j_range, "position", d_range.position);
        utl::json::try_assign(j_range, "color", d_range.color);
        utl::json::try_assign(j_range, "velocityRandom", d_range.velocityRandom);
        utl::json::try_assign(j_range, "rotationRandom", d_range.rotationRandom);

        auto& d_phys = data.physics;

        utl::json::try_assign(j_phys, "gravity", d_phys.gravity);
        utl::json::try_assign(j_phys, "resistance", d_phys.resistance);

        auto& d_flag = data.flags;

        utl::json::try_assign(j_flag, "enableRandomVelocity", d_flag.enableRandomVelocity);
        utl::json::try_assign(j_flag, "enableRandomEmit", d_flag.enableRandomEmit);
        utl::json::try_assign(j_flag, "enableRandomRotation", d_flag.enableRandomRotation);
        utl::json::try_assign(j_flag, "enableRandomScale", d_flag.enableRandomScale);
        utl::json::try_assign(j_flag, "enableScaleTransition", d_flag.enableScaleTransition);
    }

    void to_json(nlohmann::json& j, const Data& data)
    {
        j["version"]                           = 2;                            // Version 2 for this format
        j["name"]                              = data.name;

        auto& j_common  = j["common"];
        auto& d_common = data.common; 

        j_common["scaleFixed"]                  = d_common.scaleFixed;
        j_common["emitInterval"]                = d_common.emitInterval;
        j_common["emitNum"]                     = d_common.emitNum;
        j_common["emitterLifeTime"]             = d_common.emitterLifeTime;
        j_common["particleLifeTime"]            = d_common.particleLifeTime;
        j_common["scaleDelayTime"]              = d_common.scaleDelayTime;
        j_common["emitPositionFixed"]           = d_common.emitPositionFixed;
        j_common["alphaDeltaValue"]             = d_common.alphaDeltaValue;
        j_common["velocityFixed"]               = d_common.velocityFixed;

        auto& j_ranges = j["ranges"];
        auto& d_ranges = data.ranges;

        j_ranges["scale"]                       = d_ranges.scale;
        j_ranges["scaleRandom"]                 = d_ranges.scaleRandom;
        j_ranges["position"]                    = d_ranges.position;
        j_ranges["color"]                       = d_ranges.color;
        j_ranges["velocityRandom"]              = d_ranges.velocityRandom;
        j_ranges["rotationRandom"]              = d_ranges.rotationRandom;

        auto& j_phys = j["physics"];
        auto& d_phys = data.physics;

        j_phys["gravity"]                       = d_phys.gravity;
        j_phys["resistance"]                    = d_phys.resistance;

        auto& j_flag = j["flags"];
        auto& d_flag = data.flags;

        j_flag["enableRandomVelocity"]          = d_flag.enableRandomVelocity;
        j_flag["enableRandomEmit"]              = d_flag.enableRandomEmit;
        j_flag["enableRandomRotation"]          = d_flag.enableRandomRotation;
        j_flag["enableRandomScale"]             = d_flag.enableRandomScale;
        j_flag["enableScaleTransition"]         = d_flag.enableScaleTransition;
    }
}

namespace Type::ParticleEmitter::v3
{
    Data::Data(const Type::ParticleEmitter::v1::Data& rv)
    {
        *this = v2::Data(rv);
    }

    Data::Data(const Type::ParticleEmitter::v2::Data& rv)
    {
        name = rv.name;
        common.v2::Common::operator=(rv.common);
        ranges.v2::RangeData::operator=(rv.ranges);
        physics.v2::PhysicsData::operator=(rv.physics);
        flags.v2::Flags::operator=(rv.flags);
    }

    void from_json(const nlohmann::json& j, Data& data)
    {
        CheckVersion(j, data);

        utl::json::try_assign(j, "name", data.name);

        nlohmann::json j_range;  utl::json::try_assign(j, "ranges", j_range);
        nlohmann::json j_common;  utl::json::try_assign(j, "common", j_common);
        nlohmann::json j_phys;  utl::json::try_assign(j, "physics", j_phys);
        nlohmann::json j_flag;  utl::json::try_assign(j, "flags", j_flag);
        nlohmann::json j_colflo; utl::json::try_assign(j, "collisionFloor", j_colflo);

        auto& d_common = data.common;

        utl::json::try_assign(j_common, "scaleFixed", d_common.scaleFixed);
        utl::json::try_assign(j_common, "emitInterval", d_common.emitInterval);
        utl::json::try_assign(j_common, "emitNum", d_common.emitNum);
        utl::json::try_assign(j_common, "emitterLifeTime", d_common.emitterLifeTime);
        utl::json::try_assign(j_common, "particleLifeTime", d_common.particleLifeTime);
        utl::json::try_assign(j_common, "scaleDelayTime", d_common.scaleDelayTime);
        utl::json::try_assign(j_common, "emitPositionFixed", d_common.emitPositionFixed);
        utl::json::try_assign(j_common, "alphaDeltaValue", d_common.alphaDeltaValue);
        utl::json::try_assign(j_common, "velocityFixed", d_common.velocityFixed);

        auto& d_range = data.ranges;

        utl::json::try_assign(j_range, "scale", d_range.scale);
        utl::json::try_assign(j_range, "scaleRandom", d_range.scaleRandom);
        utl::json::try_assign(j_range, "position", d_range.position);
        utl::json::try_assign(j_range, "color", d_range.color);
        utl::json::try_assign(j_range, "velocityRandom", d_range.velocityRandom);
        utl::json::try_assign(j_range, "rotationRandom", d_range.rotationRandom);
        utl::json::try_assign(j_range, "velocityRandomSphere", d_range.velocityRandomSphere);

        auto& d_phys = data.physics;

        utl::json::try_assign(j_phys, "gravity", d_phys.gravity);
        utl::json::try_assign(j_phys, "resistance", d_phys.resistance);
        utl::json::try_assign(j_phys, "frictionCoef", d_phys.frictionCoef);
        utl::json::try_assign(j_phys, "smoothNoisePower", d_phys.smoothNoisePower);

        auto& d_flag = data.flags;

        utl::json::try_assign(j_flag, "enableRandomVelocity", d_flag.enableRandomVelocity);
        utl::json::try_assign(j_flag, "enableRandomEmit", d_flag.enableRandomEmit);
        utl::json::try_assign(j_flag, "enableRandomRotation", d_flag.enableRandomRotation);
        utl::json::try_assign(j_flag, "enableRandomScale", d_flag.enableRandomScale);
        utl::json::try_assign(j_flag, "enableScaleTransition", d_flag.enableScaleTransition);
        utl::json::try_assign(j_flag, "enableCollisionFloor", d_flag.enableCollisionFloor);
        utl::json::try_assign(j_flag, "velocityDistribution", d_flag.velocityDistribution);
        utl::json::try_assign(j_flag, "enableSmoothNoise", d_flag.enableSmoothNoise);

        auto& d_colFloor = data.collisionFloor;

        utl::json::try_assign(j_colflo, "elevation", d_colFloor.elevation);
        utl::json::try_assign(j_colflo, "bouncePower", d_colFloor.bounce_power);
    }

    void to_json(nlohmann::json& j, const Data& data)
    {
        j["version"]                           = 3;                            // Version 3 for this format
        j["name"]                              = data.name;

        auto& j_common  = j["common"];
        auto& d_common = data.common; 

        j_common["scaleFixed"]                  = d_common.scaleFixed;
        j_common["emitInterval"]                = d_common.emitInterval;
        j_common["emitNum"]                     = d_common.emitNum;
        j_common["emitterLifeTime"]             = d_common.emitterLifeTime;
        j_common["particleLifeTime"]            = d_common.particleLifeTime;
        j_common["scaleDelayTime"]              = d_common.scaleDelayTime;
        j_common["emitPositionFixed"]           = d_common.emitPositionFixed;
        j_common["alphaDeltaValue"]             = d_common.alphaDeltaValue;
        j_common["velocityFixed"]               = d_common.velocityFixed;

        auto& j_ranges = j["ranges"];
        auto& d_ranges = data.ranges;

        j_ranges["scale"]                       = d_ranges.scale;
        j_ranges["scaleRandom"]                 = d_ranges.scaleRandom;
        j_ranges["position"]                    = d_ranges.position;
        j_ranges["color"]                       = d_ranges.color;
        j_ranges["velocityRandom"]              = d_ranges.velocityRandom;
        j_ranges["rotationRandom"]              = d_ranges.rotationRandom;
        j_ranges["velocityRandomSphere"]        = d_ranges.velocityRandomSphere;

        auto& j_phys = j["physics"];
        auto& d_phys = data.physics;

        j_phys["gravity"]                       = d_phys.gravity;
        j_phys["resistance"]                    = d_phys.resistance;
        j_phys["frictionCoef"]                  = d_phys.frictionCoef;
        j_phys["smoothNoisePower"]              = d_phys.smoothNoisePower;

        auto& j_flag = j["flags"];
        auto& d_flag = data.flags;

        j_flag["enableRandomVelocity"]          = d_flag.enableRandomVelocity;
        j_flag["enableRandomEmit"]              = d_flag.enableRandomEmit;
        j_flag["enableRandomRotation"]          = d_flag.enableRandomRotation;
        j_flag["enableRandomScale"]             = d_flag.enableRandomScale;
        j_flag["enableScaleTransition"]         = d_flag.enableScaleTransition;
        j_flag["enableCollisionFloor"]          = d_flag.enableCollisionFloor;
        j_flag["velocityDistribution"]          = d_flag.velocityDistribution;
        j_flag["enableSmoothNoise"]             = d_flag.enableSmoothNoise;

        auto& j_colflo = j["collisionFloor"];
        auto& d_colflo = data.collisionFloor;

        j_colflo["elevation"]                   = d_colflo.elevation;
        j_colflo["bouncePower"]                 = d_colflo.bounce_power;
    }
}