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
    /// <summary>
    /// SRV マネージャの初期化を行います。
    /// </summary>
    /// <param name="pDx12">DirectX12 管理クラスへのポインタ。</param>
    void Initialize(DirectX12* pDx12);
    
    /// <summary>
    /// コマンドリストにディスクリプタヒープを設定します。
    /// </summary>
    void SetDescriptorHeaps();

    /// <summary>
    /// 新しい SRV スロットを割り当てます。
    /// </summary>
    /// <returns>割り当てられたインデックス。</returns>
    uint32_t Allocate();
    
    /// <summary>
    /// 指定したインデックスの SRV スロットを解放します。
    /// </summary>
    /// <param name="index">解放するインデックス。</param>
    void Deallocate(uint32_t index);
    
    /// <summary>
    /// 指定インデックスの CPU ディスクリプタハンドルを取得します。
    /// </summary>
    /// <param name="index">SRV インデックス。</param>
    /// <returns>CPU ディスクリプタハンドル。</returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
    
    /// <summary>
    /// 指定インデックスの GPU ディスクリプタハンドルを取得します。
    /// </summary>
    /// <param name="index">SRV インデックス。</param>
    /// <returns>GPU ディスクリプタハンドル。</returns>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

    bool IsFull() const { return currentIndex_ >= kMaxSRVCount_; }

    /// <summary>
    /// 2D テクスチャ用の SRV を作成します。
    /// </summary>
    /// <param name="index">SRV インデックス。</param>
    /// <param name="pTexture">対象テクスチャ。</param>
    /// <param name="format">フォーマット。</param>
    /// <param name="mipLevels">ミップレベル数。</param>
    void CreateForTexture2D(uint32_t index, ID3D12Resource* pTexture, DXGI_FORMAT format, UINT mipLevels);
    
    /// <summary>
    /// キューブマップ用の SRV を作成します。
    /// </summary>
    /// <param name="index">SRV インデックス。</param>
    /// <param name="pTexture">対象テクスチャ。</param>
    /// <param name="format">フォーマット。</param>
    /// <param name="mipLevels">ミップレベル数。</param>
    void CreateForCubemap(uint32_t index, ID3D12Resource* pTexture, DXGI_FORMAT format, UINT mipLevels);
    
    /// <summary>
    /// 構造化バッファ用の SRV を作成します。
    /// </summary>
    /// <param name="index">SRV インデックス。</param>
    /// <param name="pBuffer">対象バッファ。</param>
    /// <param name="numElements">要素数。</param>
    /// <param name="stride">1 要素のバイト数。</param>
    void CreateForStructuredBuffer(uint32_t index, ID3D12Resource* pBuffer, UINT numElements, UINT stride);
    
    /// <summary>
    /// テクスチャ用の UAV を作成します。
    /// </summary>
    /// <param name="index">UAV インデックス。</param>
    /// <param name="pTexture">対象テクスチャ。</param>
    /// <param name="format">フォーマット。</param>
    void CreateUAV(uint32_t index, ID3D12Resource* pTexture, DXGI_FORMAT format);
    
    /// <summary>
    /// バッファ用の UAV を作成します。
    /// </summary>
    /// <param name="index">UAV インデックス。</param>
    /// <param name="pTexture">対象リソース。</param>
    /// <param name="format">フォーマット。</param>
    /// <param name="numElements">要素数。</param>
    /// <param name="structureByteStride">1 要素のバイト数。</param>
    void CreateUAV4Buffer(uint32_t index, ID3D12Resource* pTexture, DXGI_FORMAT format, uint32_t numElements, uint32_t structureByteStride);

public: /// Setter
    /// <summary>
    /// ルートパラメータに SRV を束ねたディスクリプタテーブルを設定します。
    /// </summary>
    /// <param name="rootParameterIndex">ルートパラメータインデックス。</param>
    /// <param name="srvIndex">SRV インデックス。</param>
    void SetGraphicsRootDescriptorTable(UINT rootParameterIndex, uint32_t srvIndex);


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
    /// <summary>
    /// デバッグ用の UI を描画します。
    /// </summary>
    void ImGui();
};