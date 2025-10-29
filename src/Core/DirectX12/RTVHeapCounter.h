#pragma once

#include <wrl/client.h>
#include <d3d12.h>
#include <cstdint>
#include <vector>
#include <string>
#include <array>

/// <summary>
/// RTVヒープインデックス計算クラス
/// </summary>
class RTVHeapCounter
{
public:
    RTVHeapCounter() = default;
    ~RTVHeapCounter() = default;

    void Initialize(ID3D12Device* _device, uint32_t _numDescriptor);
    uint32_t Allocate();
    void Deallocate(uint32_t _index);


public:
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(uint32_t _index);
    ID3D12DescriptorHeap* GetRTVDescriptorHeap() const { return rtvHeap_.Get(); }


private:
    constexpr static uint32_t                       kMaxRTVCount_           = 64u;          // 最大RTV数
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    rtvHeap_                = nullptr;      // RTVヒープ
    uint32_t                                        kDescriptorSizeRTV_     = 0;            // ヒープサイズ
    uint32_t                                        currentIndex_           = 0;            // ヒープカウンタ

    std::vector<std::string>                        descriptions_           = {};           // ヒープの説明
    std::array<bool, kMaxRTVCount_>                 isAllocated_            = {};
private:
    ID3D12Device* device_ = nullptr;

};