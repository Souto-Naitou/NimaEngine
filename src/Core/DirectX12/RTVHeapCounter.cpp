#include "RTVHeapCounter.h"
#include "Helper/DX12HeapHelper.h"
#include <cassert>

void RTVHeapCounter::Initialize(ID3D12Device* _device, uint32_t _numDescriptor)
{   
    device_             = _device;
    rtvHeap_            = DX12HeapHelper::CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, _numDescriptor, false);
    kDescriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

uint32_t RTVHeapCounter::Allocate()
{
    assert(currentIndex_ < kMaxRTVCount_ && "インデックスが最大値を超えています");

    for (uint32_t i = 0; i < kMaxRTVCount_; ++i)
    {
        uint32_t idx = (currentIndex_ + i) % kMaxRTVCount_;
        if (!isAllocated_[idx])
        {
            isAllocated_[idx] = true;
            currentIndex_ = (idx + 1) % kMaxRTVCount_;
            return idx;
        }
    }

    assert(false && "RTVヒープの割り当てに失敗しました");
    return UINT32_MAX;
}

void RTVHeapCounter::Deallocate(uint32_t _index)
{
    assert(isAllocated_[_index] && "すでに開放されています。");
    isAllocated_[_index] = false;
}

D3D12_CPU_DESCRIPTOR_HANDLE RTVHeapCounter::GetRTVHandle(uint32_t _index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += kDescriptorSizeRTV_ * _index;
    return handle;
}
