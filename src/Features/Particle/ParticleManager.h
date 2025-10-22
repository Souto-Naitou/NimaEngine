#pragma once

#include <Features/Particle/Particle.h>

#include <list>
#include <memory>

/// <summary>
/// パーティクル管理クラス
/// </summary>
class ParticleManager : public EngineFeature
{
public:
    ParticleManager(ParticleManager const&) = delete;
    void operator=(ParticleManager const&) = delete;
    ParticleManager(ParticleManager const&&) = delete;
    void operator=(ParticleManager const&&) = delete;

    static ParticleManager* GetInstance()
    {
        static ParticleManager instance;
        return &instance;
    }


public:
    void Update();
    void Draw();
    void Finalize();


public:
    Particle* CreateParticle();
    void ReleaseParticle(Particle* _particle);
    void ReserveDeleteParticle(Particle* _particle);
    void ReleaseAllParticle();


private:
    ParticleManager() = default;
    ~ParticleManager() = default;


public:
    std::list<std::unique_ptr<Particle>> particles_;
    std::list<Particle*> deleteParticles_;

};