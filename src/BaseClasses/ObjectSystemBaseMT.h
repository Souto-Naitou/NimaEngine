#pragma once

#include <future>
#include <d3d12.h>
#include <wrl/client.h>
#include <Core/DirectX12/DirectX12.h>

// Forward declarations
class GameEye;

class ObjectSystemBaseMT : public EngineFeature
{
public:
    // Virtual functions
    virtual         ~ObjectSystemBaseMT() = default;      // dtor
    virtual void    Initialize();                       // Create the command list
    virtual void    PostDraw();                         // Reset the command allocator and command list
    
    // Getters
    ID3D12GraphicsCommandList*      GetCommandList();   // Get the command list
    GameEye**                       GetGlobalEye();     // Get the GameEye for global use

    // Setters
    void    SetGlobalEye(GameEye* _pGameEye);                               // Set the GameEye for global use
    void    SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE* _rtvHandle);    // Set the render target view handle

protected:
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    // Threads and DirectX objects
    std::future<void>                   worker_             = {};       // Worker thread
    ComPtr<ID3D12CommandAllocator>      commandAllocator_   = nullptr;
    ComPtr<ID3D12GraphicsCommandList>   commandList_        = nullptr;

    // Internal functions
    void CreateCommandList();

    // Pointers
    GameEye*                            pGlobalEye_         = nullptr;
    const D3D12_CPU_DESCRIPTOR_HANDLE*  rtvHandle_          = nullptr;
};