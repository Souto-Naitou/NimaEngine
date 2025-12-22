#pragma once

#include <drawable/particle/Emitter/ParticleEmitter.h>
#include <drawable/particle/Emitter/EmitterData.h>
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

    const EmitterData& LoadFile(const std::string& path);
    const EmitterData& ReloadFile(const std::string& path);
    void SaveFile(const std::string& path, const EmitterData& data);


private:
    EmitterManager() = default;
    ~EmitterManager() = default;

    void Deserialize(const json& root, EmitterData& data);
    void Serialize(json& root, const EmitterData& data);


private:
    JSONIO* pjsonio_ = nullptr;
    std::unordered_map<std::filesystem::path, EmitterData> emitterMap_;
    const std::string kDefaultPath = "EngineResources/Json/EmitterDefault.json";

};