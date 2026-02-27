#include "ParticleEmitterGroup.h"

ParticleEmitterGroup::~ParticleEmitterGroup()
{
    // emitters_の各エミッタのFinalizeを呼び出す
    for (auto& [id, emitter] : emitters_)
    {
        emitter.Finalize();
    }
}

void ParticleEmitterGroup::UpdateEmitters()
{
    for (auto& [id, emitter] : emitters_)
    {
        emitter.Update();
    }
}

void ParticleEmitterGroup::Emit(uint32_t emitterId, const Vector3& position)
{
    auto it = emitters_.find(emitterId);

    // 見つかったらエミッタの位置を更新して発生させる
    if (it != emitters_.end())
    {
        it->second.SetPosition(position);
        it->second.Emit();
    }
}

void ParticleEmitterGroup::Register(uint32_t emitterId, const EmitterConfig& config)
{
    if (emitters_.find(emitterId) != emitters_.end())
    {
        // 既に同じIDのエミッタが存在する場合は上書きする
        emitters_[emitterId] = CreateEmitter(config);
    }
    else
    {
        // 新しいIDのエミッタを登録する
        emitters_.emplace(emitterId, CreateEmitter(config));
    }
}

ParticleEmitter ParticleEmitterGroup::CreateEmitter(const EmitterConfig& config)
{
    ParticleEmitter result;
    ParticleEmitter::Params params;
    params.jsonPath = config.configPath;
    params.particle = config.pParticle;
    result.Initialize(params);
    result.SetEnableBillboard(config.enableBillboard);
    result.EnableManualMode();
    // NRVOによってムーブされるはず
    return result;
}