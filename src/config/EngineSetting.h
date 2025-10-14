#pragma once

#include <Vector4.h>
#include <dxgiformat.h>

namespace NimaEngine::Config
{
    inline constexpr DXGI_FORMAT kRenderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    inline constexpr Vector4 kEditorBGColor = { 0.03f, 0.03f, 0.03f, 1.0f };
}