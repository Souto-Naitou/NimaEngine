#include "DX12HeapHelper.h"
#include <DebugTools/Logger/Logger.h>



DX12HeapHelper::ComPtr<ID3D12DescriptorHeap> DX12HeapHelper::CreateDescriptorHeap(const ComPtr<ID3D12Device>& _device, D3D12_DESCRIPTOR_HEAP_TYPE _heapType, UINT _numDescriptors, bool _shaderVisible)
{
    ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
    descriptorHeapDesc.Type = _heapType;
    descriptorHeapDesc.NumDescriptors = _numDescriptors;
    descriptorHeapDesc.Flags = _shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT hr = _device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));

    if (FAILED(hr))
    {
        Logger::GetInstance()->LogError(
            "DX12Helper",
            __func__,
            "Failed to create descriptor heap."
        );
        assert(false && "Failed to create descriptor heap");
    }

    return descriptorHeap;
}
