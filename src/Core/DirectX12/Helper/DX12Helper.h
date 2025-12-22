#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <string>
#include <DirectXTex/DirectXTex.h>
#include <Vector4.h>
#include "../DX12Resource/DX12Resource.h"
#include "../RTVHeapCounter.h"

class DirectX12;

namespace DX12Helper
{
    using Microsoft::WRL::ComPtr;

    /// <summary>
    /// デバイスを生成する
    /// </summary>
    /// <param name="device">生成先</param>
    /// <param name="adapter">使用するアダプタ</param>
    void CreateDevice(ComPtr<ID3D12Device>& device, ComPtr<IDXGIAdapter4>& adapter);


    //#ifdef _DEBUG
    /// <summary>
    /// エラー時に停止する
    /// </summary>
    /// <param name="device">デバイス</param>
    /// <param name="infoQ">情報キュー</param>
    void PauseError(ComPtr<ID3D12Device>& device, ComPtr<ID3D12InfoQueue>& infoQ);
    //#endif // _DEBUG


    /// <summary>
    /// DepthStencilTextureリソースを生成
    /// </summary>
    /// <param name="device">デバイス</param>
    /// <param name="width">幅</param>
    /// <param name="height">高さ</param>
    /// <returns>リソース</returns>
    ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const ComPtr<ID3D12Device>& device, int32_t width, int32_t height);


    /// <summary>
    /// シェーダーをコンパイル
    /// </summary>
    /// <param name="filePath">ファイルパス</param>
    /// <param name="profile">プロファイル</param>
    /// <param name="dxcUtils">DXCユーティリティ</param>
    /// <param name="dxcCompiler">DXCコンパイラ</param>
    /// <param name="includeHandler">インクルードハンドラ</param>
    /// <returns>コンパイル結果</returns>
    ComPtr<IDxcBlob> CompileShader(
        const std::wstring& filePath,
        const wchar_t* profile,
        const ComPtr<IDxcUtils>& dxcUtils,
        const ComPtr<IDxcCompiler3>& dxcCompiler,
        const ComPtr<IDxcIncludeHandler>& includeHandler);


    /// <summary>
    /// バッファリソースを生成
    /// </summary>
    /// <param name="device">デバイス</param>
    /// <param name="sizeInBytes">バイト数</param>
    /// <returns>リソース</returns>
    ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& device, size_t sizeInBytes);
    ComPtr<ID3D12Resource> CreateBufferResource(const ComPtr<ID3D12Device>& device, size_t sizeInBytes, D3D12_RESOURCE_FLAGS flag);

    ComPtr<ID3D12Resource> CreateTextureResource(const ComPtr<ID3D12Device>& _device, const DirectX::TexMetadata& _metadata);
    
    DX12Resource CreateDX12ResourceForRender(
        const ComPtr<ID3D12Device>& device,
        RTVHeapCounter* rtvHeapCounter,
        const std::string& name
    );

    [[nodiscard]]
    ComPtr<ID3D12Resource> UploadTextureData(
        const ComPtr<ID3D12Resource>& texture,
        const DirectX::ScratchImage& mipImages,
        const ComPtr<ID3D12Device>& device,
        const ComPtr<ID3D12GraphicsCommandList>& commandList
    );
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& _descriptorHeap, uint32_t _descriptorSize, uint32_t _index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& _descriptorHeap, uint32_t _descriptorSize, uint32_t _index);

    /// <summary>
    /// 頂点リソースを生成
    /// </summary>
    /// <param name="device">デバイス</param>
    ComPtr<ID3D12Resource> CreateVertexResource(const ComPtr<ID3D12Device>& device, unsigned int countVertex);

    void ChangeStateResource(
        const ComPtr<ID3D12GraphicsCommandList>& _commandList, 
        const ComPtr<ID3D12Resource>& _resource, 
        D3D12_RESOURCE_STATES _before, 
        D3D12_RESOURCE_STATES _after
    );

    void CommandListCommonSetting(
        const DirectX12* pDx12, 
        ID3D12GraphicsCommandList* commandList, 
        const D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandle = nullptr);

    /// <summary>
    /// レンダーターゲットテクスチャリソースを生成
    /// </summary>
    /// <param name="_device">デバイス</param>
    /// <param name="_width">幅</param>
    /// <param name="_height">高さ</param>
    /// <param name="_format">フォーマット</param>
    /// <param name="_clearColor">クリア色</param>
    /// <returns>レンダーテクスチャリソース</returns>
    ComPtr<ID3D12Resource> CreateResourceForRenderTarget(
        const ComPtr<ID3D12Device>& _device,
        int32_t _width,
        int32_t _height,
        DXGI_FORMAT _format,
        const Vector4& _clearColor
    );
}