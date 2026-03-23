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
class ObjectSystemBase : public EngineFeature
{
public:
    // Virtual functions
    virtual         ~ObjectSystemBase() = default;      // dtor
    
    // Getters
    /// <summary>
    /// システム全体で共有するカメラ（GameEye）へのポインタを取得します。
    /// </summary>
    /// <returns>GameEye**</returns>
    GameEye**       GetGlobalEye();     // Get the GameEye for global use

    // Setters
    /// <summary>
    /// システム全体で共有するカメラ（GameEye）を設定します。
    /// </summary>
    /// <param name="_pGameEye">GameEye へのポインタ。</param>
    void            SetGlobalEye(GameEye* _pGameEye);                               // Set the GameEye for global use
    /// <summary>
    /// レンダーターゲットビューの CPU ハンドルを設定します。
    /// </summary>
    /// <param name="_rtvHandle">RTV CPU ハンドル。</param>
    void            SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE* _rtvHandle);    // Set the render target view handle

protected:
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    // Pointers
    GameEye*                            pGlobalEye_         = nullptr;
    const D3D12_CPU_DESCRIPTOR_HANDLE*  rtvHandle_          = nullptr;
};