#include "PostEffectHelper.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Window/Window.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/SRVManager.h>
#include <utility>
#include <config/EngineSetting.h>

void Helper::CreateRenderTexture(DirectX12* pDx12, ID3D12Device* pDevice, DX12Resource& resource, const std::string& name)
{
    const auto kFormat = NimaEngine::Config::kRenderTargetFormat;

    /// リソースの作成と初期化
    {
        auto temp = DX12Helper::CreateResourceForRenderTarget(
            pDevice,
            Window::clientWidth,
            Window::clientHeight,
            kFormat,
            NimaEngine::Config::kEditorBGColor
        );

        DX12Resource::Params param{};
        param.resource = temp;
        param.pRTVCounter = pDx12->GetRTVHeapCounter();
        param.format = kFormat;
        param.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
        param.name = name;

        resource.Initialize(param);
    }

    /// RTVの作成
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = kFormat;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    if (resource.GetRTVHandle().ptr == 0)
    {
        const auto rtvHandleIndex = pDx12->GetRTVHeapCounter()->Allocate();
        const auto rtvCPUHandle = pDx12->GetRTVHeapCounter()->GetRTVHandle(rtvHandleIndex);
        resource.SetRTV(rtvHandleIndex, rtvCPUHandle);
    }

    pDevice->CreateRenderTargetView(
        resource.GetResource().Get(),
        &rtvDesc,
        resource.GetRTVHandle()
    );
}

void Helper::CreateCommandList(ID3D12Device* pDevice, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList,  Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& Allocator)
{
    pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(Allocator.GetAddressOf()));
    pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));
}