#pragma once

#include <d3d12.h>
#include <cstdint>
#include <Core/DirectX12/DirectX12.h>
#include <wrl.h>
#include <string>
#include <array>

/// <summary>
/// SRV生成クラス 
/// </summary>
class SRVManager
{
public:
    SRVManager(const SRVManager&) = delete;
    SRVManager(const SRVManager&&) = delete;
    SRVManager& operator=(const SRVManager&) = delete;
    SRVManager& operator=(const SRVManager&&) = delete;
    static SRVManager* GetInstance()
    {
        static SRVManager instance;
        return &instance;
    }
    void Initialize(DirectX12* _pDx12);
    void SetDescriptorHeaps();

    uint32_t Allocate();
    void Deallocate(uint32_t _index);
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t _index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t _index);

    bool IsFull() const { return currentIndex_ >= kMaxSRVCount_; }

    void CreateForTexture2D(uint32_t _index, ID3D12Resource* _pTexture, DXGI_FORMAT _format, UINT _mipLevels);
    void CreateForCubemap(uint32_t _index, ID3D12Resource* _pTexture, DXGI_FORMAT _format, UINT _mipLevels);
    void CreateForStructuredBuffer(uint32_t _index, ID3D12Resource* _pBuffer, UINT _numElements, UINT _stride);
    void CreateUAV(uint32_t _index, ID3D12Resource* _pTexture, DXGI_FORMAT _format);
    void CreateUAV4Buffer(uint32_t _index, ID3D12Resource* _pTexture, DXGI_FORMAT _format, uint32_t _numElements, uint32_t _structureByteStride);

public: /// Setter
    void SetGraphicsRootDescriptorTable(UINT _rootParameterIndex, uint32_t _srvIndex);


public: /// Getter
    ID3D12DescriptorHeap* GetDescriptorHeap() const { return pDescHeap_.Get(); }


public: /// 公開定数
    static const uint32_t kMaxSRVCount_ = 512u;


private:
    SRVManager() = default;
    ~SRVManager() = default;


private:
    std::string name_ = {};

    uint32_t descriptorSize_ = 0u;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDescHeap_ = nullptr;
    uint32_t currentIndex_ = 0u;
    std::array<bool, kMaxSRVCount_> isAllocated_ = {};
    uint32_t currentSize_ = 0u;

private:
    DirectX12* pDx12_ = nullptr;


private:
    void ImGui();
};