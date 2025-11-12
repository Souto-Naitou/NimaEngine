#pragma once
#include <string>

/// <summary>
/// デバッガーのローカライズデータ
/// </summary>
namespace Localization
{
    struct Common
    {
        std::string name          = "Name";
        std::string uncategorized = "Uncategorized";
        std::string category      = "Category";
    };

    struct DebugManager
    {
        std::string window_debug         = "Debug";
        std::string window_componentList = "Components";
        std::string window_overlayFPS    = "Overlay FPS";
        std::string window_Performance   = "Performance";
    };

    struct LanguageData
    {
        Common common;
        DebugManager debugManager;
    };
}

