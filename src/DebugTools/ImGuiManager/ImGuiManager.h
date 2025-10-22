#pragma once

#ifdef _DEBUG

#include <d3d12.h>
#include <cstdint>

#include <imgui.h>
#include <string>
#include <vector>
#include <Core/DirectX12/DirectX12.h>

class DirectX12;

/// <summary>
/// ImGui管理クラス
/// </summary>
class ImGuiManager : public EngineFeature
{
public:
    // Common functions
    void Initialize();
    void BeginFrame();
    void Render();                          // 描画
    void EndFrame();                        // フレーム終了
    void Finalize();
    void OnResizedBuffers();

    // Switch options
    void EnableDocking();
    void EnableMultiViewport();
    
    // Style changes
    void StyleOriginal();
    void StylePhotoshop();
    void StyleMaterialFlat();
    void StyleFutureDark();
    void StyleComfortableDarkCyan();

private:
    // For debug
    void ImGui();

    // Internal functions
    void InitializeStyleNameArray();

    ImGuiIO*                    io_                 = nullptr;
    bool                        isChangedFont_      = false;
    uint32_t                    srvIndex_           = 0u;
    ID3D12DescriptorHeap*       srvDescHeap_        = nullptr;

    // For debug window
    std::vector<std::string>    styleNameArray_     = {};
    size_t                      idx_currentStyle_   = 0u;
};

#endif // _DEBUG