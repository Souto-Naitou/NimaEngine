#include "ConfigManager.h"

#include <config/EngineSetting.h>
#include <Utility/JSONIO/JSONIO.h>
#include <DebugTools/Logger/Logger.h>
#include <Core/Window/Window.h>

void ConfigManager::Initialize(const std::string& _cfgPath)
{
    // 初期化
    configData_ = {};
    LoadConfig(_cfgPath);
}

const cfg::ConfigData& ConfigManager::GetConfigData() const
{
    return configData_;
}

void ConfigManager::LoadConfig(const std::string& _cfgPath)
{
    bool isDirty = false;
    nlohmann::json loadedJson;

    try
    {
        // 存在するキーはそのまま読み込まれ、欠けているキーは既定値になる
        // (from_json はセクションが無くても例外を投げない)
        loadedJson = JSONIO::GetInstance()->Load(_cfgPath);
        configData_ = loadedJson;
    }
    catch (std::exception& _ex)
    {
        // ファイルが開けない/JSONが壊れている等、読み込み自体に失敗した場合のみ既定値で作り直す
        Logger::GetInstance()->LogError("ConfigManager", __func__, _ex.what());
        configData_ = {};
        isDirty = true;
    }

    // 欠けていた必須項目にのみ既定値を補う (既存の値は保持する)
    if (configData_.window_title.empty()) configData_.window_title = "Nima Engine";
    if (configData_.screen_width == 0)    configData_.screen_width = Window::clientWidth;
    if (configData_.screen_height == 0)   configData_.screen_height = Window::clientHeight;

    // 補完後の内容と読み込んだ内容を比較し、不足キーの補完があった場合のみ保存する
    // (Load 関数なので変更が無ければファイルは書き換えない)
    if (isDirty || nlohmann::json(configData_) != loadedJson)
    {
        JSONIO::GetInstance()->Save(_cfgPath, configData_);
    }

    // 実行時のみ使うパス。ファイルには書き戻さないため保存後に追加する
    configData_.texture_paths.emplace_back(NimaEngine::Config::kTextureFolderPath);
}
