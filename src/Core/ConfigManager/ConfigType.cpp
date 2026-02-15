#include "ConfigType.h"

void cfg::to_json(nlohmann::json& _j, const cfg::ConfigData& _c)
{
    auto& common = _j["settings"]["common"];
    auto& path = _j["settings"]["path"];

    common["window_title"] = _c.window_title;
    common["start_scene"] = _c.start_scene;
    common["screen_width"] = _c.screen_width;
    common["screen_height"] = _c.screen_height;

    path["model"] = _c.model_paths;
    path["texture"] = _c.texture_paths;
    path["audio"] = _c.audio_paths;
    path["particle_emitter"] = _c.particle_emitter_paths;
}

void cfg::from_json(const nlohmann::json& _j, cfg::ConfigData& _c)
{
    using namespace utl::json;
    auto& common = _j.at("settings").at("common");
    auto& path = _j.at("settings").at("path");

    try_assign(common, "window_title", _c.window_title);
    try_assign(common, "start_scene", _c.start_scene);
    try_assign(common, "screen_width", _c.screen_width);
    try_assign(common, "screen_height", _c.screen_height);

    try_assign(path, "model", _c.model_paths);
    try_assign(path, "texture", _c.texture_paths);
    try_assign(path, "audio", _c.audio_paths);
    try_assign(path, "particle_emitter", _c.particle_emitter_paths);
}
