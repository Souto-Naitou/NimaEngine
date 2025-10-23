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
    /// <param name="_pDx12">DirectX12 管理クラスへのポインタ。</param>
    void Initialize(DirectX12* _pDx12);
    
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
    /// <param name="_index">解放するインデックス。</param>
    void Deallocate(uint32_t _index);
    
    /// <summary>
    /// 指定インデックスの CPU ディスクリプタハンドルを取得します。
    /// </summary>
    /// <param name="_index">SRV インデックス。</param>
    /// <returns>CPU ディスクリプタハンドル。</returns>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t _index);
    
    /// <summary>
    /// 指定インデックスの GPU ディスクリプタハンドルを取得します。
    /// </summary>
    /// <param name="_index">SRV インデックス。</param>
    /// <returns>GPU ディスクリプタハンドル。</returns>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t _index);

    bool IsFull() const { return currentIndex_ >= kMaxSRVCount_; }

    /// <summary>
    /// 2D テクスチャ用の SRV を作成します。
    /// </summary>
    /// <param name="_index">SRV インデックス。</param>
    /// <param name="_pTexture">対象テクスチャ。</param>
    /// <param name="_format">フォーマット。</param>
    /// <param name="_mipLevels">ミップレベル数。</param>
    void CreateForTexture2D(uint32_t _index, ID3D12Resource* _pTexture, DXGI_FORMAT _format, UINT _mipLevels);
    
    /// <summary>
    /// キューブマップ用の SRV を作成します。
    /// </summary>
    /// <param name="_index">SRV インデックス。</param>
    /// <param name="_pTexture">対象テクスチャ。</param>
    /// <param name="_format">フォーマット。</param>
    /// <param name="_mipLevels">ミップレベル数。</param>
    void CreateForCubemap(uint32_t _index, ID3D12Resource* _pTexture, DXGI_FORMAT _format, UINT _mipLevels);
    
    /// <summary>
    /// 構造化バッファ用の SRV を作成します。
    /// </summary>
    /// <param name="_index">SRV インデックス。</param>
    /// <param name="_pBuffer">対象バッファ。</param>
    /// <param name="_numElements">要素数。</param>
    /// <param name="_stride">1 要素のバイト数。</param>
    void CreateForStructuredBuffer(uint32_t _index, ID3D12Resource* _pBuffer, UINT _numElements, UINT _stride);
    
    /// <summary>
    /// テクスチャ用の UAV を作成します。
    /// </summary>
    /// <param name="_index">UAV インデックス。</param>
    /// <param name="_pTexture">対象テクスチャ。</param>
    /// <param name="_format">フォーマット。</param>
    void CreateUAV(uint32_t _index, ID3D12Resource* _pTexture, DXGI_FORMAT _format);
    
    /// <summary>
    /// バッファ用の UAV を作成します。
    /// </summary>
    /// <param name="_index">UAV インデックス。</param>
    /// <param name="_pTexture">対象リソース。</param>
    /// <param name="_format">フォーマット。</param>
    /// <param name="_numElements">要素数。</param>
    /// <param name="_structureByteStride">1 要素のバイト数。</param>
    void CreateUAV4Buffer(uint32_t _index, ID3D12Resource* _pTexture, DXGI_FORMAT _format, uint32_t _numElements, uint32_t _structureByteStride);

public: /// Setter
    /// <summary>
    /// ルートパラメータに SRV を束ねたディスクリプタテーブルを設定します。
    /// </summary>
    /// <param name="_rootParameterIndex">ルートパラメータインデックス。</param>
    /// <param name="_srvIndex">SRV インデックス。</param>
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
    /// <summary>
    /// デバッグ用の UI を描画します。
    /// </summary>
    void ImGui();
};