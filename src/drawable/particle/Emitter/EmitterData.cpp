#include "EmitterData.h"

#include <Utility/JSON/jsonutl.h>
#include <Utility/JSONConvTypeFuncs/JSONConvTypeFuncs.h>

template <typename T>
static void CheckVersion(const nlohmann::json& _j,  const T& _data)
{
    uint32_t v = 0;
    utl::json::try_assign(_j, "version", v);
    if (v != _data.version)
    {
        throw std::runtime_error("Invalid version for ParticleEmitter::v1::Data: " + std::to_string(v));
    }
}

namespace Type::ParticleEmitter::v1
{
    void from_json(const nlohmann::json& _j, Data& _data)
    {
        CheckVersion(_j, _data);

        utl::json::try_assign(_j, "name", _data.name_);
        utl::json::try_assign(_j, "scaleFixed", _data.scaleFixed_);
        utl::json::try_assign(_j, "scaleRange", _data.scaleRange_);
        utl::json::try_assign(_j, "scaleRandomRange", _data.scaleRandomRange_);
        utl::json::try_assign(_j, "emitInterval", _data.emitInterval_);
        utl::json::try_assign(_j, "emitNum", _data.emitNum_);
        utl::json::try_assign(_j, "emitterLifeTime", _data.emitterLifeTime_);
        utl::json::try_assign(_j, "particleLifeTime", _data.particleLifeTime_);
        utl::json::try_assign(_j, "scaleDelayTime", _data.scaleDelayTime_);
        utl::json::try_assign(_j, "positionRange", _data.positionRange_);
        utl::json::try_assign(_j, "emitPositionFixed", _data.emitPositionFixed_);
        utl::json::try_assign(_j, "colorRange", _data.colorRange_);
        utl::json::try_assign(_j, "alphaDeltaValue", _data.alphaDeltaValue_);
        utl::json::try_assign(_j, "velocityRandomRange", _data.velocityRandomRange_);
        utl::json::try_assign(_j, "velocityFixed", _data.velocityFixed_);
        utl::json::try_assign(_j, "rotationRandomRange", _data.rotationRandomRange_);
        utl::json::try_assign(_j, "gravity", _data.gravity_);
        utl::json::try_assign(_j, "resistance", _data.resistance_);
        utl::json::try_assign(_j, "enableRandomVelocity", _data.enableRandomVelocity_);
        utl::json::try_assign(_j, "enableRandomEmit", _data.enableRandomEmit_);
        utl::json::try_assign(_j, "enableRandomRotation", _data.enableRandomRotation_);
        utl::json::try_assign(_j, "enableRandomScale", _data.enableRandomScale_);
        utl::json::try_assign(_j, "enableScaleTransition", _data.enableScaleTransition_);
    }

    void to_json(nlohmann::json& _j, const Data& _data)
    {
        _j["version"]                       = _data.version;
        _j["name"]                          = _data.name_;
        _j["scaleFixed"]                    = _data.scaleFixed_;
        _j["scaleRange"]                    = _data.scaleRange_;
        _j["scaleRandomRange"]              = _data.scaleRandomRange_;
        _j["emitInterval"]                  = _data.emitInterval_;
        _j["emitNum"]                       = _data.emitNum_;
        _j["emitterLifeTime"]               = _data.emitterLifeTime_;
        _j["particleLifeTime"]              = _data.particleLifeTime_;
        _j["scaleDelayTime"]                = _data.scaleDelayTime_;
        _j["positionRange"]                 = _data.positionRange_;
        _j["emitPositionFixed"]             = _data.emitPositionFixed_;
        _j["colorRange"]                    = _data.colorRange_;
        _j["alphaDeltaValue"]               = _data.alphaDeltaValue_;
        _j["velocityRandomRange"]           = _data.velocityRandomRange_;
        _j["velocityFixed"]                 = _data.velocityFixed_;
        _j["rotationRandomRange"]           = _data.rotationRandomRange_;
        _j["gravity"]                       = _data.gravity_;
        _j["resistance"]                    = _data.resistance_;
        _j["enableRandomVelocity"]          = _data.enableRandomVelocity_;
        _j["enableRandomEmit"]              = _data.enableRandomEmit_;
        _j["enableRandomRotation"]          = _data.enableRandomRotation_;
        _j["enableRandomScale"]             = _data.enableRandomScale_;
        _j["enableScaleTransition"]         = _data.enableScaleTransition_;
    }
}

namespace Type::ParticleEmitter::v2
{
    Data::Data(const Type::ParticleEmitter::v1::Data& _rv)
    {
        name = _rv.name_;
        common.scaleFixed = _rv.scaleFixed_;
        common.emitInterval = _rv.emitInterval_;
        common.emitNum = _rv.emitNum_;
        common.emitterLifeTime = _rv.emitterLifeTime_;
        common.particleLifeTime = _rv.particleLifeTime_;
        common.scaleDelayTime = _rv.scaleDelayTime_;
        common.emitPositionFixed = _rv.emitPositionFixed_;
        common.alphaDeltaValue = _rv.alphaDeltaValue_;
        common.velocityFixed = _rv.velocityFixed_;
        ranges.scale = _rv.scaleRange_;
        ranges.scaleRandom = _rv.scaleRandomRange_;
        ranges.position = _rv.positionRange_;
        ranges.color = _rv.colorRange_;
        ranges.velocityRandom = _rv.velocityRandomRange_;
        ranges.rotationRandom = _rv.rotationRandomRange_;
        physics.gravity = _rv.gravity_;
        physics.resistance = _rv.resistance_;
        flags.enableRandomVelocity = _rv.enableRandomVelocity_;
        flags.enableRandomEmit = _rv.enableRandomEmit_;
        flags.enableRandomRotation = _rv.enableRandomRotation_;
        flags.enableRandomScale = _rv.enableRandomScale_;
        flags.enableScaleTransition = _rv.enableScaleTransition_;
    }

    void from_json(const nlohmann::json& _j, Data& _data)
    {
        CheckVersion(_j, _data);

        utl::json::try_assign(_j, "name", _data.name);

        nlohmann::json j_range;  utl::json::try_assign(_j, "ranges", j_range);
        nlohmann::json j_common;  utl::json::try_assign(_j, "common", j_common);
        nlohmann::json j_phys;  utl::json::try_assign(_j, "physics", j_phys);
        nlohmann::json j_flag;  utl::json::try_assign(_j, "flags", j_flag);

        auto& d_common = _data.common;

        utl::json::try_assign(j_common, "scaleFixed", d_common.scaleFixed);
        utl::json::try_assign(j_common, "emitInterval", d_common.emitInterval);
        utl::json::try_assign(j_common, "emitNum", d_common.emitNum);
        utl::json::try_assign(j_common, "emitterLifeTime", d_common.emitterLifeTime);
        utl::json::try_assign(j_common, "particleLifeTime", d_common.particleLifeTime);
        utl::json::try_assign(j_common, "scaleDelayTime", d_common.scaleDelayTime);
        utl::json::try_assign(j_common, "emitPositionFixed", d_common.emitPositionFixed);
        utl::json::try_assign(j_common, "alphaDeltaValue", d_common.alphaDeltaValue);
        utl::json::try_assign(j_common, "velocityFixed", d_common.velocityFixed);

        auto& d_range = _data.ranges;

        utl::json::try_assign(j_range, "scale", d_range.scale);
        utl::json::try_assign(j_range, "scaleRandom", d_range.scaleRandom);
        utl::json::try_assign(j_range, "position", d_range.position);
        utl::json::try_assign(j_range, "color", d_range.color);
        utl::json::try_assign(j_range, "velocityRandom", d_range.velocityRandom);
        utl::json::try_assign(j_range, "rotationRandom", d_range.rotationRandom);

        auto& d_phys = _data.physics;

        utl::json::try_assign(j_phys, "gravity", d_phys.gravity);
        utl::json::try_assign(j_phys, "resistance", d_phys.resistance);

        auto& d_flag = _data.flags;

        utl::json::try_assign(j_flag, "enableRandomVelocity", d_flag.enableRandomVelocity);
        utl::json::try_assign(j_flag, "enableRandomEmit", d_flag.enableRandomEmit);
        utl::json::try_assign(j_flag, "enableRandomRotation", d_flag.enableRandomRotation);
        utl::json::try_assign(j_flag, "enableRandomScale", d_flag.enableRandomScale);
        utl::json::try_assign(j_flag, "enableScaleTransition", d_flag.enableScaleTransition);
    }

    void to_json(nlohmann::json& _j, const Data& _data)
    {
        _j["version"]                           = 2;                            // Version 2 for this format
        _j["name"]                              = _data.name;

        auto& j_common  = _j["common"];
        auto& d_common = _data.common; 

        j_common["scaleFixed"]                  = d_common.scaleFixed;
        j_common["emitInterval"]                = d_common.emitInterval;
        j_common["emitNum"]                     = d_common.emitNum;
        j_common["emitterLifeTime"]             = d_common.emitterLifeTime;
        j_common["particleLifeTime"]            = d_common.particleLifeTime;
        j_common["scaleDelayTime"]              = d_common.scaleDelayTime;
        j_common["emitPositionFixed"]           = d_common.emitPositionFixed;
        j_common["alphaDeltaValue"]             = d_common.alphaDeltaValue;
        j_common["velocityFixed"]               = d_common.velocityFixed;

        auto& j_ranges = _j["ranges"];
        auto& d_ranges = _data.ranges;

        j_ranges["scale"]                       = d_ranges.scale;
        j_ranges["scaleRandom"]                 = d_ranges.scaleRandom;
        j_ranges["position"]                    = d_ranges.position;
        j_ranges["color"]                       = d_ranges.color;
        j_ranges["velocityRandom"]              = d_ranges.velocityRandom;
        j_ranges["rotationRandom"]              = d_ranges.rotationRandom;

        auto& j_phys = _j["physics"];
        auto& d_phys = _data.physics;

        j_phys["gravity"]                       = d_phys.gravity;
        j_phys["resistance"]                    = d_phys.resistance;

        auto& j_flag = _j["flags"];
        auto& d_flag = _data.flags;

        j_flag["enableRandomVelocity"]          = d_flag.enableRandomVelocity;
        j_flag["enableRandomEmit"]              = d_flag.enableRandomEmit;
        j_flag["enableRandomRotation"]          = d_flag.enableRandomRotation;
        j_flag["enableRandomScale"]             = d_flag.enableRandomScale;
        j_flag["enableScaleTransition"]         = d_flag.enableScaleTransition;
    }
}

namespace Type::ParticleEmitter::v3
{
    Data::Data(const Type::ParticleEmitter::v1::Data& _rv)
    {
        *this = v2::Data(_rv);
    }

    Data::Data(const Type::ParticleEmitter::v2::Data& _rv)
    {
        name = _rv.name;
        common.v2::Common::operator=(_rv.common);
        ranges.v2::RangeData::operator=(_rv.ranges);
        physics.v2::PhysicsData::operator=(_rv.physics);
        flags.v2::Flags::operator=(_rv.flags);
    }

    void from_json(const nlohmann::json& _j, Data& _data)
    {
        CheckVersion(_j, _data);

        utl::json::try_assign(_j, "name", _data.name);

        nlohmann::json j_range;  utl::json::try_assign(_j, "ranges", j_range);
        nlohmann::json j_common;  utl::json::try_assign(_j, "common", j_common);
        nlohmann::json j_phys;  utl::json::try_assign(_j, "physics", j_phys);
        nlohmann::json j_flag;  utl::json::try_assign(_j, "flags", j_flag);
        nlohmann::json j_colflo; utl::json::try_assign(_j, "collisionFloor", j_colflo);

        auto& d_common = _data.common;

        utl::json::try_assign(j_common, "scaleFixed", d_common.scaleFixed);
        utl::json::try_assign(j_common, "emitInterval", d_common.emitInterval);
        utl::json::try_assign(j_common, "emitNum", d_common.emitNum);
        utl::json::try_assign(j_common, "emitterLifeTime", d_common.emitterLifeTime);
        utl::json::try_assign(j_common, "particleLifeTime", d_common.particleLifeTime);
        utl::json::try_assign(j_common, "scaleDelayTime", d_common.scaleDelayTime);
        utl::json::try_assign(j_common, "emitPositionFixed", d_common.emitPositionFixed);
        utl::json::try_assign(j_common, "alphaDeltaValue", d_common.alphaDeltaValue);
        utl::json::try_assign(j_common, "velocityFixed", d_common.velocityFixed);

        auto& d_range = _data.ranges;

        utl::json::try_assign(j_range, "scale", d_range.scale);
        utl::json::try_assign(j_range, "scaleRandom", d_range.scaleRandom);
        utl::json::try_assign(j_range, "position", d_range.position);
        utl::json::try_assign(j_range, "color", d_range.color);
        utl::json::try_assign(j_range, "velocityRandom", d_range.velocityRandom);
        utl::json::try_assign(j_range, "rotationRandom", d_range.rotationRandom);
        utl::json::try_assign(j_range, "velocityRandomSphere", d_range.velocityRandomSphere);

        auto& d_phys = _data.physics;

        utl::json::try_assign(j_phys, "gravity", d_phys.gravity);
        utl::json::try_assign(j_phys, "resistance", d_phys.resistance);
        utl::json::try_assign(j_phys, "frictionCoef", d_phys.frictionCoef);

        auto& d_flag = _data.flags;

        utl::json::try_assign(j_flag, "enableRandomVelocity", d_flag.enableRandomVelocity);
        utl::json::try_assign(j_flag, "enableRandomEmit", d_flag.enableRandomEmit);
        utl::json::try_assign(j_flag, "enableRandomRotation", d_flag.enableRandomRotation);
        utl::json::try_assign(j_flag, "enableRandomScale", d_flag.enableRandomScale);
        utl::json::try_assign(j_flag, "enableScaleTransition", d_flag.enableScaleTransition);
        utl::json::try_assign(j_flag, "enableCollisionFloor", d_flag.enableCollisionFloor);
        utl::json::try_assign(j_flag, "velocityDistribution", d_flag.velocityDistribution);

        auto& d_colFloor = _data.collisionFloor;

        utl::json::try_assign(j_colflo, "elevation", d_colFloor.elevation);
        utl::json::try_assign(j_colflo, "bouncePower", d_colFloor.bounce_power);
    }

    void to_json(nlohmann::json& _j, const Data& _data)
    {
        _j["version"]                           = 3;                            // Version 3 for this format
        _j["name"]                              = _data.name;

        auto& j_common  = _j["common"];
        auto& d_common = _data.common; 

        j_common["scaleFixed"]                  = d_common.scaleFixed;
        j_common["emitInterval"]                = d_common.emitInterval;
        j_common["emitNum"]                     = d_common.emitNum;
        j_common["emitterLifeTime"]             = d_common.emitterLifeTime;
        j_common["particleLifeTime"]            = d_common.particleLifeTime;
        j_common["scaleDelayTime"]              = d_common.scaleDelayTime;
        j_common["emitPositionFixed"]           = d_common.emitPositionFixed;
        j_common["alphaDeltaValue"]             = d_common.alphaDeltaValue;
        j_common["velocityFixed"]               = d_common.velocityFixed;

        auto& j_ranges = _j["ranges"];
        auto& d_ranges = _data.ranges;

        j_ranges["scale"]                       = d_ranges.scale;
        j_ranges["scaleRandom"]                 = d_ranges.scaleRandom;
        j_ranges["position"]                    = d_ranges.position;
        j_ranges["color"]                       = d_ranges.color;
        j_ranges["velocityRandom"]              = d_ranges.velocityRandom;
        j_ranges["rotationRandom"]              = d_ranges.rotationRandom;
        j_ranges["velocityRandomSphere"]        = d_ranges.velocityRandomSphere;

        auto& j_phys = _j["physics"];
        auto& d_phys = _data.physics;

        j_phys["gravity"]                       = d_phys.gravity;
        j_phys["resistance"]                    = d_phys.resistance;
        j_phys["frictionCoef"]                  = d_phys.frictionCoef;

        auto& j_flag = _j["flags"];
        auto& d_flag = _data.flags;

        j_flag["enableRandomVelocity"]          = d_flag.enableRandomVelocity;
        j_flag["enableRandomEmit"]              = d_flag.enableRandomEmit;
        j_flag["enableRandomRotation"]          = d_flag.enableRandomRotation;
        j_flag["enableRandomScale"]             = d_flag.enableRandomScale;
        j_flag["enableScaleTransition"]         = d_flag.enableScaleTransition;
        j_flag["enableCollisionFloor"]          = d_flag.enableCollisionFloor;
        j_flag["velocityDistribution"]          = d_flag.velocityDistribution;

        auto& j_colflo = _j["collisionFloor"];
        auto& d_colflo = _data.collisionFloor;

        j_colflo["elevation"]                   = d_colflo.elevation;
        j_colflo["bouncePower"]                 = d_colflo.bounce_power;
    }
}