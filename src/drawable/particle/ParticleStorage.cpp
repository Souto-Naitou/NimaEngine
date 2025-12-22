#include "./ParticleStorage.h"
#include <cassert>

void ParticleStorage::Update()
{
    size_t numDelete = deleteParticles_.size();
    for (size_t i = 0; i < numDelete; ++i)
    {
        deleteParticles_.remove_if([&](Particle* p)
        {
            if (!p->IsAbleDelete()) return false;

            ReleaseParticle(p);
            return true;
        });
    }

    for (auto& particle : particles_)
    {
        particle->Update();
    }
}

void ParticleStorage::Finalize()
{
    ReleaseAllParticle();
}

Particle* ParticleStorage::CreateParticle()
{
    auto& ref = particles_.emplace_back(std::make_unique<Particle>());
    ref->SetDirectX12(pDx12_);
    return ref.get();
}

void ParticleStorage::ReleaseParticle(Particle* particle)
{
    for (auto itr = particles_.begin(); itr != particles_.end(); ++itr)
    {
        auto ptr = itr->get();
        if (ptr == particle)
        {
            ptr->Finalize();
            particles_.erase(itr);
            return;
        }
    }
}

void ParticleStorage::ReserveDeleteParticle(Particle* particle)
{
    deleteParticles_.push_back(particle);
}

void ParticleStorage::ReleaseAllParticle()
{
    for (auto& particle : particles_)
    {
        particle->Finalize();
    }

    particles_.clear();
    deleteParticles_.clear();
}
