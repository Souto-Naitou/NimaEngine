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
    /// <summary>
    /// 全パーティクルの更新を行います。
    /// </summary>
    void Update();
    /// <summary>
    /// 全パーティクルの描画を行います。
    /// </summary>
    void Draw();
    /// <summary>
    /// 全パーティクルの破棄など後始末を行います。
    /// </summary>
    void Finalize();


public:
    /// <summary>
    /// 新しいパーティクルを生成し、管理下に置きます。
    /// </summary>
    /// <returns>生成したパーティクル。</returns>
    Particle* CreateParticle();
    /// <summary>
    /// 即時にパーティクルを破棄します。
    /// </summary>
    /// <param name="_particle">対象パーティクル。</param>
    void ReleaseParticle(Particle* _particle);
    /// <summary>
    /// フレーム終端での破棄を予約します。
    /// </summary>
    /// <param name="_particle">対象パーティクル。</param>
    void ReserveDeleteParticle(Particle* _particle);
    /// <summary>
    /// 管理下の全パーティクルを破棄します。
    /// </summary>
    void ReleaseAllParticle();


private:
    ParticleManager() = default;
    ~ParticleManager() = default;


public:
    std::list<std::unique_ptr<Particle>> particles_;
    std::list<Particle*> deleteParticles_;

};