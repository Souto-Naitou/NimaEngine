#pragma once

#include <DebugTools/ImGuiManager/ImGuiManager.h>
#include <Core/DirectX12/TextureManager.h>

class TextureSelectWidget
{
public:
    static TextureSelectWidget* GetInstance() { static TextureSelectWidget instance; return &instance; }

    TextureSelectWidget() = default;
    TextureSelectWidget(const TextureSelectWidget&) = delete;
    TextureSelectWidget& operator=(const TextureSelectWidget&) = delete;
    TextureSelectWidget(TextureSelectWidget&&) = delete;
    TextureSelectWidget& operator=(TextureSelectWidget&&) = delete;

    void SetImGuiManager(ImGuiManager* pImGuiManager) { pImGuiManager_ = pImGuiManager; }

    bool DrawSelector(const TextureManager::TextureData*& outTextureData, float maxWidth);

private:
    ImVec2 buttonSizeMax_ = { 64.0f, 64.0f };
    ImGuiManager* pImGuiManager_ = nullptr;
};