#pragma once

#ifdef _DEBUG

#include <d3d12.h>
#include <cstdint>

#include <imgui.h>
#include <string>
#include <vector>
#include <Core/DirectX12/DirectX12.h>
#include <utility>

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
    void PostDraw();                        // フレーム終了
    void Finalize();
    void OnResizedBuffers();

    /// <summary>
    /// ImGui::Imageで使用する画像リソースの追加
    /// リソースの状態遷移を実行するため登録が必須
    /// </summary>
    /// <param name="resource">リソース</param>
    void AddImageResource(DX12Resource* resource);

    void RemoveImageResource(DX12Resource* resource);

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

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>   commandList_        = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>      commandAllocator_   = nullptr;

    std::vector<std::pair<DX12Resource*, D3D12_RESOURCE_STATES>>  imageResources_     = {};   // ImGui::Imageで使用するリソースの配列

    // For debug window
    std::vector<std::string>    styleNameArray_     = {};
    size_t                      idx_currentStyle_   = 0u;
};

#endif // _DEBUG