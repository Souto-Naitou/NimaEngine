#include "PostEffectHelper.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Win32/WinSystem.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/SRVManager.h>
#include <utility>
#include <config/EngineSetting.h>

void Helper::CreateRenderTexture(DirectX12* _pDx12, ID3D12Device* _pDevice, DX12Resource& resource, const std::string& name)
{
    const auto kFormat = NimaEngine::Config::kRenderTargetFormat;

    /// リソースの作成と初期化
    {
        auto temp = DX12Helper::CreateResourceForRenderTarget(
            _pDevice,
            WinSystem::clientWidth,
            WinSystem::clientHeight,
            kFormat,
            NimaEngine::Config::kEditorBGColor
        );

        resource.Initialize(temp, D3D12_RESOURCE_STATE_RENDER_TARGET, kFormat, name);
    }

    /// RTVの作成
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = kFormat;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    if (resource.GetRTVHandle().ptr == 0)
    {
        const auto rtvHandleIndex = _pDx12->GetRTVHeapCounter()->Allocate("PostEffectRTV");
        const auto rtvCPUHandle = _pDx12->GetRTVHeapCounter()->GetRTVHandle(rtvHandleIndex);
        resource.SetRTV(rtvHandleIndex, rtvCPUHandle);
    }

    _pDevice->CreateRenderTargetView(
        resource.GetResource().Get(),
        &rtvDesc,
        resource.GetRTVHandle()
    );
}

void Helper::CreateSRV(DX12Resource& resource)
{
    auto pSRVManager = SRVManager::GetInstance();

    /// 割り当て
    auto index = pSRVManager->Allocate();
    pSRVManager->CreateForTexture2D(
        index,
        resource.GetResource().Get(),
        NimaEngine::Config::kRenderTargetFormat,
        1
    );

    /// ラッパークラスにSRV情報をセット
    auto cpuHandle = pSRVManager->GetCPUDescriptorHandle(index);
    auto gpuHandle = pSRVManager->GetGPUDescriptorHandle(index);
    resource.SetSRV(index, cpuHandle, gpuHandle);
}

void Helper::CreateCommandList(ID3D12Device* _pDevice, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& _commandList,  Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& _Allocator)
{
    _pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(_Allocator.GetAddressOf()));
    _pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _Allocator.Get(), nullptr, IID_PPV_ARGS(_commandList.GetAddressOf()));
}