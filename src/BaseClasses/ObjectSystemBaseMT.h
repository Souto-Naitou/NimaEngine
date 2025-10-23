#pragma once

#include <future>
#include <d3d12.h>
#include <wrl/client.h>
#include <Core/DirectX12/DirectX12.h>

// Forward declarations
class GameEye;

/// <summary>
/// オブジェクトシステムの基底クラス(マルチスレッド)
/// </summary>
class ObjectSystemBaseMT : public EngineFeature
{
public:
    // Virtual functions
    virtual         ~ObjectSystemBaseMT() = default;      // dtor
    /// <summary>
    /// コマンドリストなどを生成し、システムを初期化します。
    /// </summary>
    virtual void    Initialize();                       // Create the command list
    /// <summary>
    /// コマンドアロケータとコマンドリストのリセットなど、描画後処理を行います。
    /// </summary>
    virtual void    PostDraw();                         // Reset the command allocator and command list
    
    // Getters
    /// <summary>
    /// このシステムが使用するコマンドリストを取得します。
    /// </summary>
    /// <returns>ID3D12GraphicsCommandList*</returns>
    ID3D12GraphicsCommandList*      GetCommandList();   // Get the command list
    /// <summary>
    /// システム全体で共有するカメラ（GameEye）へのポインタを取得します。
    /// </summary>
    /// <returns>GameEye**</returns>
    GameEye**                       GetGlobalEye();     // Get the GameEye for global use

    // Setters
    /// <summary>
    /// システム全体で共有するカメラ（GameEye）を設定します。
    /// </summary>
    /// <param name="_pGameEye">GameEye へのポインタ。</param>
    void    SetGlobalEye(GameEye* _pGameEye);                               // Set the GameEye for global use
    /// <summary>
    /// レンダーターゲットビューの CPU ハンドルを設定します。
    /// </summary>
    /// <param name="_rtvHandle">RTV CPU ハンドル。</param>
    void    SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE* _rtvHandle);    // Set the render target view handle

protected:
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    // Threads and DirectX objects
    std::future<void>                   worker_             = {};       // Worker thread
    ComPtr<ID3D12CommandAllocator>      commandAllocator_   = nullptr;
    ComPtr<ID3D12GraphicsCommandList>   commandList_        = nullptr;

    // Internal functions
    /// <summary>
    /// コマンドアロケータとコマンドリストを生成します。
    /// </summary>
    void CreateCommandList();

    // Pointers
    GameEye*                            pGlobalEye_         = nullptr;
    const D3D12_CPU_DESCRIPTOR_HANDLE*  rtvHandle_          = nullptr;
};