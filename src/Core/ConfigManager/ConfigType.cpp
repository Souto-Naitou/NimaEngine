#include "ConfigType.h"

void cfg::to_json(nlohmann::json& _j, const cfg::ConfigData& _c)
{
    auto& common = _j["settings"]["common"];
    auto& path = _j["settings"]["path"];
    auto& particleEmitter = _j["settings"]["particle_emitter"];

    common["window_title"] = _c.window_title;
    common["start_scene"] = _c.start_scene;
    common["screen_width"] = _c.screen_width;
    common["screen_height"] = _c.screen_height;

    particleEmitter["default_config_filename"] = _c.particle_emitter_default_config_filename;

    path["model"] = _c.model_paths;
    path["texture"] = _c.texture_paths;
    path["audio"] = _c.audio_paths;
    path["particle_emitter"] = _c.particle_emitter_paths;
}

void cfg::from_json(const nlohmann::json& _j, cfg::ConfigData& _c)
{
    using namespace utl::json;

    // セクションが存在しなくても例外を投げず、空オブジェクトを返す
    static const nlohmann::json empty = nlohmann::json::object();
    auto sub = [](const nlohmann::json& _parent, const char* _key) -> const nlohmann::json&
    {
        auto itr = _parent.find(_key);
        return itr != _parent.end() ? itr.value() : empty;
    };

    const auto& settings = sub(_j, "settings");
    const auto& common = sub(settings, "common");
    const auto& path = sub(settings, "path");
    const auto& particleEmitter = sub(settings, "particle_emitter");

    try_assign(common, "window_title", _c.window_title);
    try_assign(common, "start_scene", _c.start_scene);
    try_assign(common, "screen_width", _c.screen_width);
    try_assign(common, "screen_height", _c.screen_height);

    try_assign(particleEmitter, "default_config_filename", _c.particle_emitter_default_config_filename);

    try_assign(path, "model", _c.model_paths);
    try_assign(path, "texture", _c.texture_paths);
    try_assign(path, "audio", _c.audio_paths);
    try_assign(path, "particle_emitter", _c.particle_emitter_paths);
}
