#pragma once
#include <wrl/client.h>
#include <d3d12.h>

namespace DX12HeapHelper
{
    using Microsoft::WRL::ComPtr;

    /// <summary>
    /// ディスクリプタヒープの生成
    /// </summary>
    /// <param name="_device">デバイス</param>
    /// <param name="_heapType">ヒープタイプ</param>
    /// <param name="_numDescriptors">ディスクリプタの数</param>
    /// <param name="_shaderVisible">シェーダーが参照可能か</param>
    /// <returns></returns>
    ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
        const ComPtr<ID3D12Device>& _device,
        D3D12_DESCRIPTOR_HEAP_TYPE _heapType,
        UINT _numDescriptors,
        bool _shaderVisible);
}