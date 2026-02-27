#pragma once
#include <Vector3.h>
#include <drawable/particle/emitter/ParticleEmitter.h>
#include <drawable/particle/Particle.h>
#include <string>
#include <unordered_map>
#include <cstdint>

class ParticleEmitterGroup
{
public:
    struct EmitterConfig
    {
        Particle* pParticle = nullptr;
        std::string configPath = {};
        bool enableBillboard = true;
    };

    ParticleEmitterGroup() = default;
    ~ParticleEmitterGroup();

    /// <summary>
    /// エミッターを更新します
    /// </summary>
    void UpdateEmitters();

    /// <summary>
    /// 指定されたエミッターを使用して、指定された位置で放出します
    /// </summary>
    /// <param name="emitterId">使用するエミッターのID</param>
    /// <param name="position">放出する位置</param>
    void Emit(uint32_t emitterId, const Vector3& position);

    /// <summary>
    /// エミッターを登録します。
    /// </summary>
    /// <param name="emitterId">エミッターID</param>
    /// <param name="config">エミッターの設定</param>
    void Register(uint32_t emitterId, const EmitterConfig& config);

private:
    ParticleEmitter CreateEmitter(const EmitterConfig& config);
    std::unordered_map<uint32_t, ParticleEmitter> emitters_;
};