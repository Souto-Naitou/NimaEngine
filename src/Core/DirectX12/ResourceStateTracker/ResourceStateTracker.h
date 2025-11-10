#pragma once
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgiformat.h>

/// <summary>
/// リソースの状態を管理するクラス
/// </summary>
class ResourceStateTracker
{
public:
    void Initialize(
        Microsoft::WRL::ComPtr<ID3D12Resource> pResource,
        D3D12_RESOURCE_STATES state,
        DXGI_FORMAT format
    );

    inline D3D12_RESOURCE_STATES   GetState() const    { return state_; }
    inline const DXGI_FORMAT&      GetFormat() const   { return format_; }

    void Reset();

    /// <summary>
    /// リソースの状態を変更します。
    /// </summary>
    /// <param name="commandList">コマンドリスト</param>
    /// <param name="newState">遷移先ステート</param>
    /// <returns>遷移前ステート</returns>
    D3D12_RESOURCE_STATES ChangeState(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState);

private:
    Microsoft::WRL::ComPtr<ID3D12Resource>  pResource_  = nullptr;
    D3D12_RESOURCE_STATES                   state_      = D3D12_RESOURCE_STATE_COMMON;
    DXGI_FORMAT                             format_     = DXGI_FORMAT_UNKNOWN;
};