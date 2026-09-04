#pragma once

#ifdef _DEBUG
#include <DebugTools/ImGuiManager/ImGuiManager.h>
#endif // _DEBUG
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

    #ifdef _DEBUG
    void SetImGuiManager(ImGuiManager* pImGuiManager) { pImGuiManager_ = pImGuiManager; }
    #else
    void SetImGuiManager(void*) {}
    #endif // _DEBUG

    bool DrawSelector(const TextureManager::TextureData*& outTextureData, float maxWidth);

private:
    #ifdef _DEBUG
    ImVec2 buttonSizeMax_ = { 64.0f, 64.0f };
    ImGuiManager* pImGuiManager_ = nullptr;
    #endif // _DEBUG
};