#include "EmitterManager.h"

using EmitterData = Type::ParticleEmitter::v3::Data;

const EmitterData& EmitterManager::LoadFile(const std::string& path)
{
    if (!pjsonio_) pjsonio_ = JSONIO::GetInstance();

    EmitterData data = {};
    auto& root = pjsonio_->Load(path);

    Deserialize(root, data);

    emitterMap_[path] = data;

    return emitterMap_[path];
}

const EmitterData& EmitterManager::ReloadFile(const std::string& path)
{
    if (!pjsonio_) pjsonio_ = JSONIO::GetInstance();
    pjsonio_->Unload(path);
    return LoadFile(path);
}

void EmitterManager::SaveFile(const std::string& path, const EmitterData& data)
{
    pjsonio_->Save(path, data);
}

void EmitterManager::Deserialize(const json& root, EmitterData& data)
{
    int version = root["version"].get<int>();
    switch (version)
    {
    case 1:
        data = Type::ParticleEmitter::v1::Data(root);
        break;
    case 2:
        data = Type::ParticleEmitter::v2::Data(root);
        break;
    case 3:
        data = Type::ParticleEmitter::v3::Data(root);
        break;

    default: break;
    }
}

void EmitterManager::Serialize(json& root, const EmitterData& data)
{
    root = data;
}
