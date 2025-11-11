#include "EmitterManager.h"

using EmitterData = Type::ParticleEmitter::v3::Data;

const EmitterData& EmitterManager::LoadFile(const std::string& _path)
{
    if (!pjsonio_) pjsonio_ = JSONIO::GetInstance();

    EmitterData data = {};
    auto& root = pjsonio_->Load(_path);

    Deserialize(root, data);

    emitterMap_[_path] = data;

    return emitterMap_[_path];
}

const EmitterData& EmitterManager::ReloadFile(const std::string& _path)
{
    if (!pjsonio_) pjsonio_ = JSONIO::GetInstance();
    pjsonio_->Unload(_path);
    return LoadFile(_path);
}

void EmitterManager::SaveFile(const std::string& _path, const EmitterData& _data)
{
    pjsonio_->Save(_path, _data);
}

void EmitterManager::Deserialize(const json& _root, EmitterData& _data)
{
    int version = _root["version"].get<int>();
    switch (version)
    {
    case 1:
        _data = Type::ParticleEmitter::v1::Data(_root);
        break;
    case 2:
        _data = Type::ParticleEmitter::v2::Data(_root);
        break;
    case 3:
        _data = Type::ParticleEmitter::v3::Data(_root);
        break;

    default: break;
    }
}

void EmitterManager::Serialize(json& _root, const EmitterData& _data)
{
    _root = _data;
}
