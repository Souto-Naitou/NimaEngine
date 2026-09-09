#pragma once

#include <Interfaces/IGameEye.h>
#include <drawable/line/LineSystem.h>
#include <drawable/sprite/SpriteSystem.h>
#include <drawable/object3d/Object3dSystem.h>
#include <drawable/particle/ParticleSystem.h>

struct GameEyeDelivery
{
    GameEyeDelivery(IGameEye* pGameEye)
    {
        Object3dSystem::GetInstance()->SetGlobalEye(pGameEye);
        SpriteSystem::GetInstance()->SetGlobalEye(pGameEye);
        LineSystem::GetInstance()->SetGlobalEye(pGameEye);
        ParticleSystem::GetInstance()->SetGlobalEye(pGameEye);
    }

    ~GameEyeDelivery()
    {
        Object3dSystem::GetInstance()->SetGlobalEye(nullptr);
        SpriteSystem::GetInstance()->SetGlobalEye(nullptr);
        LineSystem::GetInstance()->SetGlobalEye(nullptr);
        ParticleSystem::GetInstance()->SetGlobalEye(nullptr);
    }
};