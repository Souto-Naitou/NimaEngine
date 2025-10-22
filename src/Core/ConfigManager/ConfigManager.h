#pragma once

#include "./ConfigType.h"
#include <Common/SingletonPattern.h>
#include <string>

/// <summary>
/// 設定ファイルの読み込み・保持をするクラス
/// </summary>
class ConfigManager : public SingletonPattern<ConfigManager>
{
    friend class SingletonPattern<ConfigManager>;
public:
    // Common functions
    void Initialize(const std::string& _cfgPath);
    
    // Getters
    const cfg::ConfigData& GetConfigData() const;

private:
    // Internal functions
    void LoadConfig(const std::string& _cfgPath);

    // Config data
    cfg::ConfigData configData_ = {};
};