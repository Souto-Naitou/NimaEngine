#include "RTVHeapCounter.h"
#include "Helper/DX12HeapHelper.h"
#include <cassert>

void RTVHeapCounter::Initialize(ID3D12Device* device, uint32_t numDescriptor)
{   
    device_             = device;
    rtvHeap_            = DX12HeapHelper::CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, numDescriptor, false);
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

void RTVHeapCounter::Deallocate(uint32_t index)
{
    assert(isAllocated_[index] && "すでに開放されています。");
    isAllocated_[index] = false;
}

D3D12_CPU_DESCRIPTOR_HANDLE RTVHeapCounter::GetRTVHandle(uint32_t index)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += kDescriptorSizeRTV_ * index;
    return handle;
}
