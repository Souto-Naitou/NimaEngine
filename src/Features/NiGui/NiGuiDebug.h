#pragma once

#include <NiGui/Interface/NiGui_IDebug.h>

/// <summary>
/// NiGuiデバッグクラス
/// </summary>
class NiGuiDebug : public INiGuiDebug
{
public:
    NiGuiDebug() = default;
    void DrawDebugUI();

private:
    bool isDisplayComponentDataWindow_ = false;

    BaseDrawData foundComponentData_ = {};
    bool isSearchComponentData_ = false;
    bool isFoundComponentData_ = false;
    uint32_t frameCountPressReturn_ = 0;

private:
    void DisplayComponentDataWindow();
    void DisplayMenuBar();
};