#include "RTVHeapCounter.h"
#include "Helper/DX12HeapHelper.h"

void RTVHeapCounter::Initialize(ID3D12Device* _device, uint32_t _numDescriptor)
{   
    device_             = _device;
    rtvHeap_            = DX12HeapHelper::CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, _numDescriptor, false);
    kDescriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

uint32_t RTVHeapCounter::Allocate(const std::string& _description)
{
    D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += kDescriptorSizeRTV_ * rtvHeapCount_;

    rtvHandles_.emplace_back(handle);
    descriptions_.emplace_back(_description);
    
    return rtvHeapCount_++;
}

D3D12_CPU_DESCRIPTOR_HANDLE RTVHeapCounter::GetRTVHandle(uint32_t _index)
{
    if (_index >= rtvHeapCount_)
    {
        return D3D12_CPU_DESCRIPTOR_HANDLE{};
    }
    return rtvHandles_[_index];
}
