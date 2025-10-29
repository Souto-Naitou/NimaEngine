#pragma once

#include <Vector4.h>
#include <dxgiformat.h>

/// <summary>
/// NimaEngineのグローバル設定
/// </summary>
namespace NimaEngine::Config
{
    inline constexpr DXGI_FORMAT kRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    inline constexpr Vector4 kEditorBGColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    inline constexpr const char* kTextureFolderPath = "EngineResources/Textures";
    inline constexpr const char* kDefaultTexturePath = "EngineResources/Textures/white1x1.png";
}