#pragma once
#include <string>

/// <summary>
/// デバッガーのローカライズデータ
/// </summary>
namespace Localization
{
    struct _Common
    {
        std::string name          = "Name";
        std::string uncategorized = "Uncategorized";
        std::string category      = "Category";
    };

    struct _DebugManager
    {
        std::string window_debug         = "Debug";
        std::string window_componentList = "Components";
        std::string window_overlayFPS    = "Overlay FPS";
        std::string window_Performance   = "Performance";
    };

    struct LanguageData
    {
        _Common common;
        _DebugManager debugManager;
    };
}

