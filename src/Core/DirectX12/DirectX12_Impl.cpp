/// DirectX12クラスの実装部分

#include "DirectX12.h"

#include <Utility/ConvertString/ConvertString.h>
#include <cassert>
#include <format>
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/DirectX12/SRVManager.h>

/// ImGui
#ifdef _DEBUG
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#endif // _DEBUG

#include <dxcapi.h>
#include <Core/Window/Window.h>
#include <config/EngineSetting.h>
#include "Helper/DX12HeapHelper.h"
#pragma comment(lib, "dxcompiler.lib")

const uint32_t DirectX12::kMaxSRVCount_ = 512ui32;

void DirectX12::SetGameWindowRect(D3D12_VIEWPORT viewport)
{
    viewport_ = viewport;
    //pSRVManager_->Deallocate(gameWndSrvIndex_);
    //gameScreenResource_.Reset();

    //CreateGameScreenResource();
}

void DirectX12::ChooseAdapter()
{
    for (uint32_t i = 0;
        dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND;
        i++)
    {
        /// アダプタ情報を取得
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr_ = useAdapter_->GetDesc3(&adapterDesc);
        if (FAILED(hr_))
        {
            pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to get adapter description");
            assert(false && "Failed to get adapter description");
        }

        // ソフトウェアアダプタではなければ採用
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
        {
            // 出力
            pLogger_->LogInfo(__FILE__, __FUNCTION__, std::format("Adapter : {}", ConvertString(adapterDesc.Description)));
            break;
        }
        useAdapter_ = nullptr;
    }

    // アダプタが見つからなかった場合
    if (!useAdapter_)
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Adapter not found");
        assert(useAdapter_ && "Failed to find adapter");
    }
}

void DirectX12::CreateCommandResources()
{
    /// コマンドキューを生成
    pLogger_->LogInfo(__FILE__, __FUNCTION__, "Begin to create command queue");
    hr_ = device_->CreateCommandQueue(&commandQueueDesc_, IID_PPV_ARGS(&commandQueue_));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create command queue");
        assert(false && "Failed to create command queue");

        return;
    }
    else
    {
        pLogger_->LogInfo(__FILE__, __FUNCTION__, "Command queue creation completed");
    }


    /// コマンドアロケータを生成
    pLogger_->LogInfo(__FILE__, __FUNCTION__, "Begin to create command allocator");
    hr_ = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create command allocator");
        assert(false && "Failed to create command allocator");

        return;
    }
    else
    {
        pLogger_->LogInfo(__FILE__, __FUNCTION__, "Command allocator creation completed");
    }


    /// コマンドリストを生成
    pLogger_->LogInfo(__FILE__, __FUNCTION__, "Begin to create command list");
    hr_ = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create command list");
        assert(false && "Failed to create command list");

        return;
    }
    else
    {
        pLogger_->LogInfo(__FILE__, __FUNCTION__, "Command list creation completed");
    }
    this->AddCommandList(CommandListType::Common, commandList_.Get());
}

void DirectX12::CreateSwapChainAndResource()
{
    /// スワップチェーンの設定
    swapChainDesc_ = {};
    swapChainDesc_.Width            = Window::clientWidth;                   // 画面の幅。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc_.Height           = Window::clientHeight;                  // 画面の高さ。ウィンドウのクライアント領域を同じものにしておく
    swapChainDesc_.Format           = NimaEngine::Config::kRenderTargetFormat; // 色の形式
    swapChainDesc_.SampleDesc.Count = 1;                                        // マルチサンプルしない
    swapChainDesc_.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;          // 描画のターゲットとして利用する
    swapChainDesc_.BufferCount      = 2;                                        // ダブルバッファ
    swapChainDesc_.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_DISCARD;            // モニタにうつしたら、中身を破棄


    /// 生成
    hr_ = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), hwnd_, &swapChainDesc_, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create swap chain");
        assert(false && "Failed to create swap chain");
        return;
    }
    else
    {
        pLogger_->LogInfo(__FILE__, __FUNCTION__, "Swap chain creation completed");
    }


    /// Descriptorのサイズを取得 (動的に変わらないもの)
    kDescriptorSizeSRV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    kDescriptorSizeRTV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    kDescriptorSizeDSV = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    /// RTVHeapの生成
    rtvHeapCounter_ = std::make_unique<RTVHeapCounter>();

    /// ディスクリプタヒープの生成も行う
    rtvHeapCounter_->Initialize(device_.Get(), 64);
    dsvDescriptorHeap_ = DX12HeapHelper::CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);


    /// SwapChainからResourceを引っ張ってくる

    Microsoft::WRL::ComPtr<ID3D12Resource> tempResources[2] = {};

    hr_ = swapChain_->GetBuffer(0, IID_PPV_ARGS(&tempResources[0]));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to get resource from swap chain [0]");
        assert(false && "Failed to get resource from swap chain [0]");
        return;
    }
    hr_ = swapChain_->GetBuffer(1, IID_PPV_ARGS(&tempResources[1]));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to get resource from swap chain [1]");
        assert(false && "Failed to get resource from swap chain [1]");
        return;
    }
    pLogger_->LogInfo(__FILE__, __FUNCTION__, "Resource acquisition from swap chain completed");

    swapChain_->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709);

    /// RTVの設定
    rtvDesc_.Format = NimaEngine::Config::kRenderTargetFormat;
    rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;


    /// RTVの生成
    rtvHeapIndex_[0] = rtvHeapCounter_->Allocate();
    rtvHeapIndex_[1] = rtvHeapCounter_->Allocate();

    // RTVハンドルの取得
    rtvHandles_[0] = rtvHeapCounter_->GetRTVHandle(rtvHeapIndex_[0]);
    rtvHandles_[1] = rtvHeapCounter_->GetRTVHandle(rtvHeapIndex_[1]);

    // 2つのRTVを作成
    device_->CreateRenderTargetView(tempResources[0].Get(), &rtvDesc_, rtvHandles_[0]);
    device_->CreateRenderTargetView(tempResources[1].Get(), &rtvDesc_, rtvHandles_[1]);

    DX12Resource::Params params{};
    params.format = NimaEngine::Config::kRenderTargetFormat;
    params.resource = tempResources[0];
    params.state = D3D12_RESOURCE_STATE_PRESENT;
    params.name = "SwapchainResource0";
    params.pRTVCounter = rtvHeapCounter_.get();
    swapChainResources_[0].Initialize(params);

    params.resource = tempResources[1];
    params.name = "SwapchainResource1";
    swapChainResources_[1].Initialize(params);
}

void DirectX12::CreateGameScreenResource()
{
    /// リソースの生成
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Width            = static_cast<UINT64>(viewport_.Width);       // 幅
    resourceDesc.Height           = static_cast<UINT>(viewport_.Height);        // 高さ
    resourceDesc.MipLevels        = 1;                                          // mipmapの数
    resourceDesc.DepthOrArraySize = 1;                                          // 奥行き or 配列Textureの配列数
    resourceDesc.Format           = NimaEngine::Config::kRenderTargetFormat;    // フォーマット
    resourceDesc.SampleDesc.Count = 1;                                          // サンプリング数
    resourceDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;         // 2DTexture
    resourceDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // UAVを使うためのフラグ
    resourceDesc.Layout           = D3D12_TEXTURE_LAYOUT_UNKNOWN;               // テクスチャのレイアウト

    D3D12_HEAP_PROPERTIES heapProperties{};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;                          // VRAMに

    {
        Microsoft::WRL::ComPtr<ID3D12Resource> tempResource = nullptr;

        device_->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(tempResource.GetAddressOf())
        );

        DX12Resource::Params params{};
        params.format = NimaEngine::Config::kRenderTargetFormat;
        params.resource = tempResource;
        params.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        params.name = "GameScreenComputed";
        params.pRTVCounter = rtvHeapCounter_.get();

        gameScreenResource_.Initialize(params);
    }

    /// SRVの生成
    {
        SRVManager* psrvm = SRVManager::GetInstance();
        auto gameWndSrvIndex = psrvm->Allocate();
        psrvm->CreateForTexture2D(
            gameWndSrvIndex,
            gameScreenResource_.GetResource().Get(),
            NimaEngine::Config::kRenderTargetFormat,
            1
        );

        auto cpuHandle = psrvm->GetCPUDescriptorHandle(gameWndSrvIndex);
        auto gpuHandle = psrvm->GetGPUDescriptorHandle(gameWndSrvIndex);

        gameScreenResource_.SetSRV(gameWndSrvIndex, cpuHandle, gpuHandle);
    }

    /// UAVの生成
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Alignment          = 0;
    textureDesc.Width              = static_cast<UINT64>(viewport_.Width);
    textureDesc.Height             = static_cast<UINT>(viewport_.Height);
    textureDesc.DepthOrArraySize   = 1;
    textureDesc.MipLevels          = 1;
    textureDesc.Format             = NimaEngine::Config::kRenderTargetFormat;
    textureDesc.SampleDesc.Count   = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    D3D12_HEAP_PROPERTIES heapPropertiesUAV = {};
    heapPropertiesUAV.Type = D3D12_HEAP_TYPE_DEFAULT;

    Microsoft::WRL::ComPtr<ID3D12Resource> tempUAVResource = nullptr;

    device_->CreateCommittedResource(
        &heapPropertiesUAV,
        D3D12_HEAP_FLAG_NONE,
        &textureDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr,
        IID_PPV_ARGS(tempUAVResource.GetAddressOf())
    );

    DX12Resource::Params uavParams{};
    uavParams.format = NimaEngine::Config::kRenderTargetFormat;
    uavParams.resource = tempUAVResource;
    uavParams.state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    uavParams.name = "GameScreenComputed";
    uavParams.pRTVCounter = rtvHeapCounter_.get();

    gameScreenComputed_.Initialize(uavParams);
}

void DirectX12::CreateDSVAndSettingState()
{
    /// リソースの生成
    Microsoft::WRL::ComPtr<ID3D12Resource> tempResource = nullptr;
    tempResource = DX12Helper::CreateDepthStencilTextureResource(device_, Window::clientWidth, Window::clientHeight);

    DX12Resource::Params params{};
    params.format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    params.state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    params.resource = tempResource;
    params.name = "DepthStencilResource";
    params.pRTVCounter = rtvHeapCounter_.get();

    depthStencilResource_.Initialize(params);

    /// DSVの生成
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // フォーマット
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2DTexture
    device_->CreateDepthStencilView(depthStencilResource_.GetResource().Get(), &dsvDesc, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());
}

void DirectX12::CreateFenceAndEvent()
{
    /// 初期値0でFenceを作る
    hr_ = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create fence");
        assert(false && "Failed to create fence");
    }
    else
    {
        pLogger_->LogInfo(__FILE__, __FUNCTION__, "Fence creation completed");
    }


    /// FenceのSignalを待つためのイベントを作成する
    fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
    assert(fenceEvent_ != nullptr);
}

void DirectX12::SetViewportAndScissorRect()
{
    /// ビューポート
    viewport_.Width    = static_cast<FLOAT>(Window::clientWidth);
    viewport_.Height   = static_cast<FLOAT>(Window::clientHeight);
    viewport_.TopLeftX = 0;
    viewport_.TopLeftY = 0;
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;

    /// シザー矩形
    // 基本的にビューポートと同じ矩形が構成されるようにする
    scissorRect_.left   = 0;
    scissorRect_.right  = Window::clientWidth;
    scissorRect_.top    = 0;
    scissorRect_.bottom = Window::clientHeight;
}

void DirectX12::CreateDirectXShaderCompiler()
{
    /// dxcCompilerを初期化
    hr_ = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to initialize DXC");
        assert(false && "Failed to create DXCUtils");
    }
    hr_ = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to initialize DXCCompiler");
        assert(false && "Failed to create DXCCompiler");
    }

    /// includeに対応するため
    hr_ = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create include handler");
        assert(false && "Failed to create include handler");
    }

    pLogger_->LogInfo(__FILE__, __FUNCTION__, "DXC initialization completed");
}

void DirectX12::CreateD2D1Factory()
{
    /// Direct2Dのファクトリを生成
    hr_ = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2dFactory_));
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create D2D1 factory");
        assert(false && "Failed to create D2D1 factory");
    }
    else
    {
        pLogger_->LogInfo(__FILE__, __FUNCTION__, "D2D1 factory creation completed");
    }
}

void DirectX12::CreateD3D11Device()
{
    #ifdef _DEBUG
    UINT d3d11flags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    #else
    UINT d3d11flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    #endif // _DEBUG

    hr_ = D3D11On12CreateDevice(device_.Get(), d3d11flags, nullptr, 0, reinterpret_cast<IUnknown**>(commandQueue_.GetAddressOf()), 1, 0, &d3d11Device_, &d3d11On12DeviceContext_, nullptr);
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create D3D11 device");
        assert(false && "Failed to create D3D11 device");
    }
    else
    {
        pLogger_->LogInfo(__FILE__, __FUNCTION__, "D3D11 device creation completed");
    }

    hr_ = d3d11Device_.As(&d3d11On12Device_);
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create D3D11On12 device");
        assert(false && "Failed to create D3D11On12 device");
    }
    else
    {
        pLogger_->LogInfo(__FILE__, __FUNCTION__, "D3D11On12 device creation completed");
    }
}

void DirectX12::CreateID2D1DeviceContext()
{
    hr_ = d3d11On12Device_.As(&dxgiDevice_);
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create DXGI device");
        assert(false && "Failed to create DXGI device");
    }

    hr_ = d2dFactory_->CreateDevice(dxgiDevice_.Get(), &d2dDevice_);
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create D2D device");
        assert(false && "Failed to create D2D device");
    }

    hr_ = d2dDevice_->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dDeviceContext_.ReleaseAndGetAddressOf());
    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create D2D device context");
        assert(false && "Failed to create D2D device context");
    }

    pLogger_->LogInfo(__FILE__, __FUNCTION__, "D2D device context creation completed");
}

void DirectX12::CreateD2DRenderTarget()
{
    D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
    const UINT dpi = GetDpiForWindow(hwnd_);
    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
            static_cast<float>(dpi),
            static_cast<float>(dpi)
        );

    for (UINT i = 0u; i < 2; ++i)
    {
        Microsoft::WRL::ComPtr<ID3D11Resource> wrappedBackBuffer = nullptr;
        hr_ = d3d11On12Device_->CreateWrappedResource(
            swapChainResources_[i].GetResource().Get(),
            &d3d11Flags,
            swapChainResources_[i].GetStateTracker().GetState(),
            D3D12_RESOURCE_STATE_PRESENT,
            IID_PPV_ARGS(wrappedBackBuffer.ReleaseAndGetAddressOf())
        );

        if (FAILED(hr_))
        {
            pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create D3D11 wrapped back buffer");
            assert(false && "Failed to create D2D render target");

            return;
        }

        Microsoft::WRL::ComPtr<IDXGISurface> dxgiSurface = nullptr;
        hr_ = wrappedBackBuffer.As(&dxgiSurface);
        if (FAILED(hr_))
        {
            pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create DXGI surface");
            assert(false && "Failed to create DXGI surface");

            return;
        }

        Microsoft::WRL::ComPtr<ID2D1Bitmap1> d2dRenderTarget = nullptr;
        hr_ = d2dDeviceContext_->CreateBitmapFromDxgiSurface(
            dxgiSurface.Get(),
            &bitmapProperties,
            d2dRenderTarget.ReleaseAndGetAddressOf()
        );
        if (FAILED(hr_))
        {
            pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create D2D render target");
            assert(false && "Failed to create D2D render target");

            return;
        }

        d3d11WrappedBackBuffers_[i] = wrappedBackBuffer;
        d2dRenderTargets_[i] = d2dRenderTarget;
    }

    pLogger_->LogInfo(__FILE__, __FUNCTION__, "D2D render target creation completed");
}

void DirectX12::ResizeBuffers()
{
    WaitForGPU();

    for (auto& [key, func] : mapFuncOnResizeBefore_)
    {
        func();
    }

    d3d11Device_.Reset();
    d3d11On12DeviceContext_.Reset();
    d3d11On12Device_.Reset();
    dxgiDevice_.Reset();
    d2dDevice_.Reset();
    d2dDeviceContext_.Reset();

    for (UINT i = 0; i < 2; ++i)
    {
        d2dRenderTargets_[i].Reset();
        d3d11WrappedBackBuffers_[i].Reset();
        swapChainResources_[i].Reset();
    }

    depthStencilResource_.Reset();

    gameScreenResource_.Reset();
    gameScreenComputed_.Reset();

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    swapChain_->GetDesc1(&desc);
    hr_ = swapChain_->ResizeBuffers(
        2,
        Window::clientWidth,
        Window::clientHeight,
        desc.Format,
        desc.Flags
    );

    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to resizing buffers");
        assert(false);
    }
    else
    {
        pLogger_->LogInfo(__FILE__, __FUNCTION__, "Success resizing buffers");
    }

    for (UINT i = 0; i < 2; ++i)
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> tempResource = nullptr;
        swapChain_->GetBuffer(i, IID_PPV_ARGS(&tempResource));

        DX12Resource::Params params{};
        params.format = desc.Format;
        params.resource = tempResource;
        params.state = D3D12_RESOURCE_STATE_PRESENT;
        params.name = "SwapchainResource" + std::to_string(i);
        params.pRTVCounter = rtvHeapCounter_.get();

        swapChainResources_[i].Initialize(params);
        device_->CreateRenderTargetView(swapChainResources_[i].GetResource().Get(), nullptr, rtvHandles_[i]);
    }

    backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

    // ビューポート・シザーも更新
    viewport_.Width     = static_cast<float>(Window::clientWidth);
    viewport_.Height    = static_cast<float>(Window::clientHeight);
    scissorRect_.right  = static_cast<LONG>(Window::clientWidth);
    scissorRect_.bottom = static_cast<LONG>(Window::clientHeight);

    auto& cl = commandLists_[DirectX12::CommandListType::Common].begin()->second;
    cl->RSSetViewports(1, &viewport_);
    cl->RSSetScissorRects(1, &scissorRect_);

    CreateDSVAndSettingState();
    CreateD3D11Device();
    CreateID2D1DeviceContext();
    CreateD2DRenderTarget();
    CreateGameScreenResource();

    for (auto& [key, func] : mapFuncOnResizeAfter_)
    {
        func();
    }
}

void DirectX12::WaitForGPU()
{
    /// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
    commandQueue_->Signal(fence_.Get(), fenceValue_);

    /// GPUがすでに指定したフェンス値を超えていれば、待機せずに処理を進める
    if (fence_->GetCompletedValue() < fenceValue_)
    {
        // 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
        fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
        // イベントを待つ
        WaitForSingleObject(fenceEvent_, INFINITE);
    }
}
