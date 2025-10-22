#pragma once

#include <Features/Particle/Emitter/ParticleEmitter.h>
#include <Features/Particle/Emitter/EmitterData.h>
#include <Utility/JSONIO/JSONIO.h>
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <string>

/// <summary>
/// エミッタマネージャー
/// </summary>
class EmitterManager
{
public:
    using json = nlohmann::json;
    using EmitterData = Type::ParticleEmitter::v3::Data;

    EmitterManager(const EmitterManager&) = delete;
    EmitterManager& operator=(const EmitterManager&) = delete;
    EmitterManager(EmitterManager&&) = delete;
    EmitterManager& operator=(EmitterManager&&) = delete;

    static EmitterManager* GetInstance()
    {
        static EmitterManager instance;
        return &instance;
    }

    const EmitterData& LoadFile(const std::string& _path);
    const EmitterData& ReloadFile(const std::string& _path);
    void SaveFile(const std::string& _path, const EmitterData& _data);


private:
    EmitterManager() = default;
    ~EmitterManager() = default;

    void Deserialize(const json& _root, EmitterData& _data);
    void Serialize(json& _root, const EmitterData& _data);


private:
    JSONIO* pjsonio_ = nullptr;
    std::unordered_map<std::filesystem::path, EmitterData> emitterMap_;
    const std::string kDefaultPath = "EngineResources/Json/EmitterDefault.json";

};