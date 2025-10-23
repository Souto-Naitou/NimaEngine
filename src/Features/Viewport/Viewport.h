#pragma once

#include <Common/HRESULT_ASSERT.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/SRVManager.h>
#include <d3d12.h>
#include <wrl.h>
#ifdef _DEBUG
#include <imgui.h>
#endif //_DEBUG
#include <Vector2.h>

/// <summary>
/// ImGui上にビューポートを表示するクラス
/// </summary>
class Viewport : public EngineFeature
{
public:
    Viewport() = default;
    ~Viewport() = default;

    /// <summary>
    /// 必要なリソースを生成し、ビューポート機能を初期化します。
    /// </summary>
    void Initialize();
    /// <summary>
    /// 入力テクスチャからビューポート用に画像処理を行います。
    /// </summary>
    void Compute();
    /// <summary>
    /// スワップチェイン等のリサイズに伴うリソースの再生成を行います。
    /// </summary>
    void OnResizedBuffers();
    /// <summary>
    /// ImGui 上にビューポートウィンドウを描画します。
    /// </summary>
    void DrawWindow();


public: /// Getter
    uint32_t GetOutputSRVIndex() const { return outputSRVIndex_; }
    Vector2 GetViewportPos() const { return vpPos_; }
    Vector2 GetViewportSize() const { return vpSize_; }


private:
    /// <summary>
    /// ルートシグネチャを作成します。
    /// </summary>
    void CreateRootSignature();
    /// <summary>
    /// パイプラインステートオブジェクトを作成します。
    /// </summary>
    void CreatePSO();
    /// <summary>
    /// SRV を作成します。
    /// </summary>
    void CreateSRV();
    /// <summary>
    /// UAV を作成します。
    /// </summary>
    void CreateUAV();


private:
    HRESULT_ASSERT hr_ = {};

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> csBlob_ = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_ = nullptr;
    D3D12_VIEWPORT viewport_ = {};

    uint32_t inputSRVIndex_ = 0u;
    uint32_t outputSRVIndex_ = 0u;
    uint32_t outputUAVIndex_ = 0u;
    
    static constexpr wchar_t kComputeShaderPath_[] = L"EngineResources/Shaders/WithoutAlpha.CS.hlsl";

    Vector2 windowPos_ = { 0, 0 };
    Vector2 vpPos_ = { 0, 0 };
    Vector2 vpSize_ = { 0, 0 };
    #ifdef _DEBUG
    ImVec2  nextContentRegionSize_ = {};
    #endif 

    // DirectX 12 objects
    ID3D12Device*               device_         = nullptr;
    IDxcUtils*                  dxcUtils_       = nullptr;
    IDxcCompiler3*              dxcCompiler_    = nullptr;
    IDxcIncludeHandler*         includeHandler_ = nullptr;
    ID3D12GraphicsCommandList*  commandList_    = nullptr;
    DX12Resource*               inputTexture_   = nullptr;
    DX12Resource*               outputTexture_  = nullptr;
    SRVManager*                 pSRVManager_    = nullptr;
};