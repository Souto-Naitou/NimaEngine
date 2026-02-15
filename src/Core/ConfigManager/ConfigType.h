#pragma once
#include <string>
#include <list>

#include <nlohmann/json.hpp>
#include <Utility/JSON/jsonutl.h>

namespace cfg
{
    /// <summary>
    /// 設定データ (ファイルから読み込まれる)
    /// </summary>
    struct ConfigData
    {
        std::string window_title;
        uint32_t screen_width;
        uint32_t screen_height;
        std::string start_scene;

        std::list<std::string> model_paths;
        std::list<std::string> texture_paths;
        std::list<std::string> audio_paths;
        std::list<std::string> particle_emitter_paths;
    };

    void to_json(nlohmann::json& _j, const cfg::ConfigData& _c);
    void from_json(const nlohmann::json& _j, cfg::ConfigData& _c);
}
