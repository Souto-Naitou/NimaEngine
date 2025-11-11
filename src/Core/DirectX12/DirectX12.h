#pragma once

#include "./Framerate.h"
#include <DebugTools/ReakChecker.h>
#include <DebugTools/Logger/Logger.h>
#include "./RTVHeapCounter.h"
#include "./DX12Resource/DX12Resource.h"

#include <d3d12.h>
#include <d3d11.h>
#include <dxgi1_6.h>

#ifdef WINDOWS_LEAN_AND_MEAN

#undef WINDOWS_LEAN_AND_MEAN

#endif // WINDOWS_LEAN_AND_MEAN


#include <Windows.h>
#include <dxcapi.h>

#include <d3d11on12.h>


/// DirectWrite
#include <d2d1_3.h>
#include <dwrite_3.h>

#include <vector>
#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <list>

class SRVManager;

/// <summary>
/// DirectX12管理クラス
/// </summary>
class DirectX12
{
public:
    // ポストエフェクトや合成用
    constexpr static DXGI_FORMAT kHDRFormat_ = DXGI_FORMAT_R16G16B16A16_FLOAT;

    enum class CommandListType
    {
        Common,
        DrawableObject,
        PostEffectExecuter,
        Viewport,
        ImGui,
    };

public:
    DirectX12() = default;
    ~DirectX12();

    void Initialize();

    void OnResizedWindow();

    void NewFrame();
    void CommandExecute();
    void DisplayFrame();
    void WaitForGPU();
    void CopyFromRTV(ID3D12GraphicsCommandList* _commandList);

    /// <summary>
    /// ゲーム画面リソースの生成
    /// </summary>
    void CreateGameScreenResource();

    static const uint32_t kMaxSRVCount_;


public: /// Getter
    ID3D12Device*                                           GetDevice() const                           { return device_.Get(); }
    ID3D12GraphicsCommandList*                              GetCommandList() const                      { return commandList_.Get(); }
    ID3D12GraphicsCommandList*                              GetCommandListLast() const;
    IDxcUtils*                                              GetDxcUtils() const                         { return dxcUtils_.Get(); }
    IDxcCompiler3*                                          GetDxcCompiler() const                      { return dxcCompiler_.Get(); }
    IDxcIncludeHandler*                                     GetIncludeHandler() const                   { return includeHandler_.Get(); }

    uint32_t                                                GetDescriptorSizeSRV() const                { return kDescriptorSizeSRV; }

    int32_t                                                 GetNumUploadedTexture() const               { return numUploadedTexture; }

    const D3D12_CPU_DESCRIPTOR_HANDLE*                      GetRTVHandle() const                        { return rtvHandles_; }
    const DXGI_SWAP_CHAIN_DESC1&                            GetSwapChainDesc() const                    { return swapChainDesc_; }
    auto                                                    GetDSVDescriptorHeap() const                { return dsvDescriptorHeap_.Get(); }
    auto                                                    GetRTVDescriptorHeap() const                { return rtvHeapCounter_->GetRTVDescriptorHeap(); }
    RTVHeapCounter*                                         GetRTVHeapCounter() const                   { return rtvHeapCounter_.get(); }

    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>&               GetSRVHandlesGPUList()                      { return srvHandlesGPUList_; }
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>&               GetSRVHandlesCPUList()                      { return srvHandlesCPUList_; }
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>&    GetTextureResources()                       { return textureResources_; }
    IDXGIDevice*                                            GetDXGIDevice()                             { return dxgiDevice_.Get(); }
    ID2D1Factory3*                                          GetD2D1Factory()                            { return d2dFactory_.Get(); }
    ID2D1Device2*                                           GetDirect2dDevice()                         { return d2dDevice_.Get(); }
    ID2D1DeviceContext2*                                    GetDirect2dDeviceContext()                  { return d2dDeviceContext_.Get(); }
    const D3D12_VIEWPORT&                                   GetViewport() const                         { return viewport_; }
    uint32_t                                                GetGameWndSRVIndex() const                  { return gameScreenResource_.GetSRVIndex(); }
    uint32_t                                                GetBackBufferIndex() const                  { return backBufferIndex_; }
    ID3D11Resource*                                         GetD3D11WrappedBackBuffer(uint32_t _index)  { return d3d11WrappedBackBuffers_[_index].Get(); }
    ID2D1Bitmap1*                                           GetD2D1RenderTarget(uint32_t _index)        { return d2dRenderTargets_[_index].Get(); }
    ID3D11On12Device*                                       GetD3D11On12Device()                        { return d3d11On12Device_.Get(); }
    ID3D11DeviceContext*                                    GetD3D11On12DeviceContext()                 { return d3d11On12DeviceContext_.Get(); }
    DX12Resource*                                           GetGameScreenResource()                     { return &gameScreenResource_; }
    DX12Resource*                                           GetGameScreenComputed()                     { return &gameScreenComputed_; }
    const D3D12_RECT&                                       GetScissorRect() const                      { return scissorRect_; }
    DX12Resource*                                           GetDepthStencilResource()                   { return &depthStencilResource_; }

    void SetGameWindowRect(D3D12_VIEWPORT viewport);
    void SetGameWndSRVIndex(uint32_t index) { gameWndSrvIndexComputed_ = index; }
    void AddCommandList(CommandListType type, ID3D12GraphicsCommandList* commandList, uint32_t order = 0);
    void RemoveCommandList(CommandListType type, ID3D12GraphicsCommandList* commandList);

    /// Resizeイベント用関数登録
    void AddOnResizeBefore(const std::string& key, std::function<void()> func) { mapFuncOnResizeBefore_[key] = func; }
    void AddOnResizeAfter(const std::string& key, std::function<void()> func) { mapFuncOnResizeAfter_[key] = func; }

    void DeleteOnResizeBefore(const std::string& key) { mapFuncOnResizeBefore_.erase(key); }
    void DeleteOnResizeAfter(const std::string& key) { mapFuncOnResizeAfter_.erase(key); }

private:
    static D3DResourceLeakChecker                           leakchecker;
    HRESULT                                                 hr_                             = 0;
    HWND                                                    hwnd_                           = {};
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>                srvHandlesGPUList_              = {};           // SRVハンドルリスト(GPU)
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>                srvHandlesCPUList_              = {};           // SRVハンドルリスト(CPU)
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>     textureResources_               = {};           // テクスチャリソース
    Microsoft::WRL::ComPtr<ID3D12Debug1>                    debugController_                = nullptr;      // デバッグコントローラ
    Microsoft::WRL::ComPtr<IDXGIFactory7>                   dxgiFactory_                    = nullptr;      // DXGIファクトリ
    Microsoft::WRL::ComPtr<IDXGIAdapter4>                   useAdapter_                     = nullptr;      // 使うアダプタ
    Microsoft::WRL::ComPtr<ID3D12InfoQueue>                 infoQueue_                      = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>              commandQueue_                   = nullptr;      // コマンドキュー
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>          commandAllocator_               = nullptr;      // コマンドアロケータ
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>       commandList_                    = nullptr;      // コマンドアロケータ
    Microsoft::WRL::ComPtr<IDXGISwapChain4>                 swapChain_                      = nullptr;      // スワップチェーン
    DX12Resource                                            swapChainResources_[2]          = {};           // スワップチェーンリソース
    Microsoft::WRL::ComPtr<ID3D12Fence>                     fence_                          = nullptr;      // フェンス
    std::unique_ptr<RTVHeapCounter>                         rtvHeapCounter_                 = nullptr;      // RTVヒープカウンタ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>            dsvDescriptorHeap_              = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>            descriptorHeaps_                = nullptr;
    DX12Resource                                            depthStencilResource_           = {};           // 深度ステンシルリソース
    DX12Resource                                            gameScreenResource_             = {};           // ゲーム画面(ImGuiを含まない)リソース
    DX12Resource                                            gameScreenComputed_             = {};           // ゲーム画面(コンピュートシェーダー後)リソース
    Microsoft::WRL::ComPtr<IDxcUtils>                       dxcUtils_                       = nullptr;
    Microsoft::WRL::ComPtr<IDxcCompiler3>                   dxcCompiler_                    = nullptr;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler>              includeHandler_                 = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Device>                    device_                         = nullptr;      // デバイス

    // directwrite用
    Microsoft::WRL::ComPtr<ID3D11Device>                    d3d11Device_                    = nullptr;      // D3D11デバイス
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>             d3d11On12DeviceContext_         = nullptr;      // D3D11デバイスコンテキスト
    Microsoft::WRL::ComPtr<ID3D11On12Device>                d3d11On12Device_                = nullptr;      // D3D11On12デバイス
    Microsoft::WRL::ComPtr<ID2D1Factory3>                   d2dFactory_                     = nullptr;      // Direct2D
    Microsoft::WRL::ComPtr<IDXGIDevice>                     dxgiDevice_                     = nullptr;      // DXGIデバイス
    Microsoft::WRL::ComPtr<ID2D1Device2>                    d2dDevice_                      = nullptr;      // d2d1デバイス
    Microsoft::WRL::ComPtr<ID2D1DeviceContext2>             d2dDeviceContext_               = nullptr;      // d2d1デバイスコンテキスト
    std::array<Microsoft::WRL::ComPtr<ID3D11Resource>, 2>   d3d11WrappedBackBuffers_        = {};           // D3D11のラップされたバックバッファ
    std::array<Microsoft::WRL::ComPtr<ID2D1Bitmap1>, 2>     d2dRenderTargets_               = {};           // D2D1のレンダーターゲット

    /// コマンドリスト
    std::map<CommandListType, uint32_t>                     nextCLOrder_                = {};           // コマンドリストの実行順番カウンタ
    using CLList = std::map<uint32_t, ID3D12GraphicsCommandList*>;
    std::map<CommandListType, CLList>                       commandLists_                   = {};           // コマンドリストのリスト

    D3D12_RESOURCE_BARRIER                                  barrier_                        = {};
    D3D12_COMMAND_QUEUE_DESC                                commandQueueDesc_               = {};           // コマンドキューの設定
    DXGI_SWAP_CHAIN_DESC1                                   swapChainDesc_                  = {};           // スワップチェーンの設定
    D3D12_RENDER_TARGET_VIEW_DESC                           rtvDesc_                        = {};           // RTVの設定
    HANDLE                                                  fenceEvent_                     = {};
    uint64_t                                                fenceValue_                     = 0u;           // フェンス値
    D3D12_VIEWPORT                                          viewport_                       = {};           // ビューポート
    D3D12_RECT                                              scissorRect_                    = {};           // シザーレクト
    D3D12_CPU_DESCRIPTOR_HANDLE                             rtvHandles_[2]                  = {};           // RTVハンドル
    uint32_t                                                rtvHeapIndex_[2]                = {};           // RTVヒープインデックス
    uint32_t                                                gameWndSrvIndexComputed_        = 0;
    float                                                   clearColor_[4]                  = { 0.2f, 0.2f, 0.4f, 1.0f };
    uint32_t                                                backBufferIndex_                = 0u;
    uint32_t                                                kDescriptorSizeSRV              = 0u;
    uint32_t                                                kDescriptorSizeRTV              = 0u;
    uint32_t                                                kDescriptorSizeDSV              = 0u;
    int32_t                                                 numUploadedTexture              = 0;
    int32_t                                                 padding                         = 0;
    bool                                                    isResized_                      = false;
    std::unordered_map<std::string, std::function<void()>>  mapFuncOnResizeBefore_          = {};
    std::unordered_map<std::string, std::function<void()>>  mapFuncOnResizeAfter_           = {};


private:

    /// <summary>
    /// アダプタの選択
    /// </summary>
    void ChooseAdapter();


    /// <summary>
    /// Command[List/Queue/Allocator]を生成する
    /// </summary>
    void CreateCommandResources();


    /// <summary>
    /// スワップチェーンの生成
    /// </summary>
    void CreateSwapChainAndResource();


    /// <summary>
    /// DepthStencilViewの生成
    /// </summary>
    void CreateDSVAndSettingState();


    /// <summary>
    /// フェンスの生成とイベントの生成
    /// </summary>
    void CreateFenceAndEvent();


    /// <summary>
    /// ビューポートとシザー矩形の設定
    /// </summary>
    void SetViewportAndScissorRect();


    /// <summary>
    /// DirectXShaderCompilerの生成
    /// </summary>
    void CreateDirectXShaderCompiler();


    /// <summary>
    /// Direct2Dのファクトリの生成
    /// </summary>
    void CreateD2D1Factory();


    /// <summary>
    /// D3D11デバイスの生成
    /// </summary>
    void CreateD3D11Device();


    /// <summary>
    /// デバイスコンテキストの生成
    /// </summary>
    void CreateID2D1DeviceContext();


    /// <summary>
    /// レンダーターゲットの生成
    /// </summary>
    void CreateD2DRenderTarget();

    void ResizeBuffers();

private: /// 他クラスのインスタンス(シングルトンなど)
    FrameRate* pFramerate_ = nullptr;
    SRVManager* pSRVManager_ = nullptr;
    Logger* pLogger_ = nullptr;
};

class EngineFeature
{
public:
    EngineFeature() = default;
    virtual ~EngineFeature() = default;

    void        SetDirectX12(DirectX12* _pDx12);
    DirectX12*  GetDirectX12();

protected:
    DirectX12* pDx12_ = nullptr; 
};