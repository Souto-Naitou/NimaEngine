#include "DirectX12.h"

#include <Utility/ConvertString/ConvertString.h>        // wideとの互換
#include <Core/Window/Window.h>                          // Window関連
#include <Core/DirectX12/Helper/DX12Helper.h>           // ヘルパー
#include <Core/DirectX12/SRVManager.h>                  // SRV管理

#ifdef _DEBUG
#include <imgui_impl_dx12.h>
#endif // DEBUG


#include <cassert>
#include <config/EngineSetting.h>

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <dxcapi.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")


/// <summary>
/// 初期化呼出
/// </summary>


void DirectX12::Initialize()
{
    const bool underPix = (GetModuleHandleW(L"WinPixGpuCapturer.dll") != nullptr);

    /// デバッグコントローラの設定
    #ifdef _DEBUG
    if (!underPix)
    {
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_))))
        {
            // デバッグレイヤーを有効化する
            debugController_->EnableDebugLayer();
            // GPU側もチェックする
            debugController_->SetEnableGPUBasedValidation(TRUE);
        }
    }
    #endif // _DEBUG

    pSRVManager_ = SRVManager::GetInstance();

    pFramerate_ = FrameRate::GetInstance();
    pFramerate_->Initialize();

    pLogger_ = Logger::GetInstance();

    // ウィンドウハンドルを取得
    hwnd_ = Window::GetInstance()->GetHwnd();

    hr_ = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));

    if (FAILED(hr_))
    {
        pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to create DXGI Factory.");
        return;
    }


    /// アダプタ
    ChooseAdapter();


    /// デバイス生成
    DX12Helper::CreateDevice(device_, useAdapter_);


    /// エラー時停止処理
    //#ifdef _DEBUG
    if (!underPix) DX12Helper::PauseError(device_, infoQueue_);
    //#endif // _DEBUG


    /// 出力ウィンドウに初期化完了を出力
    pLogger_->LogInfo(
        "DirectX12",
        __func__,
        "DirectX12 Initialized."
    );


    /// コマンド系を生成
    CreateCommandResources();


    /// スワップチェーンの生成とRTVのリソース生成
    CreateSwapChainAndResource();


    /// DSVの生成とステートの設定
    CreateDSVAndSettingState();


    /// フェンスとイベントの生成
    CreateFenceAndEvent();


    /// ビューポートとシザー矩形の設定
    SetViewportAndScissorRect();


    /// DXCの初期化
    CreateDirectXShaderCompiler();


    /// D3D11デバイス群の生成
    CreateD3D11Device();


    /// Direct2Dのファクトリの生成
    CreateD2D1Factory();


    /// D2D1デバイスコンテキストの生成
    CreateID2D1DeviceContext();


    /// D2D1レンダーターゲットの生成
    CreateD2DRenderTarget();
}

void DirectX12::OnResizedWindow()
{
    this->ResizeBuffers();
    isResized_ = false;
}

void DirectX12::NewFrame()
{
    auto* cl = commandLists_[DirectX12::CommandListType::Common].begin()->second;

    // リソースバリアの設定
    swapChainResources_[backBufferIndex_].GetStateTracker().ChangeState(cl, D3D12_RESOURCE_STATE_RENDER_TARGET);

    /// 描画先のRTV/DSVの設定
    cl->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex_], false, nullptr);

    // 画面全体のクリア
    cl->ClearRenderTargetView(rtvHandles_[backBufferIndex_], &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    cl->RSSetViewports(1, &viewport_);            // Viewportを設定
    cl->RSSetScissorRects(1, &scissorRect_);      // Scissorを設定
}

void DirectX12::CommandExecute()
{
    ID3D12GraphicsCommandList* clLastExecution = commandList_.Get();
    if (!commandLists_.empty()) clLastExecution = this->GetCommandListLast();

    swapChainResources_[backBufferIndex_].GetStateTracker().ChangeState(clLastExecution, D3D12_RESOURCE_STATE_PRESENT);

    /// GPUにコマンドリストを実行させる
    std::vector<ID3D12CommandList*> commandLists = {};
    for(auto& pair : commandLists_)
    {
        CLList clList = pair.second;
        for (auto& cl : clList)
        {
            hr_ = cl.second->Close();

            if (FAILED(hr_))
            {
                pLogger_->LogError(__FILE__, __FUNCTION__, "Failed to close command list.");
                assert(false && "Failed to close command list");
            }

            commandLists.push_back(cl.second);
        }
    }
    commandQueue_->ExecuteCommandLists(static_cast<UINT>(commandLists.size()), commandLists.data());
}

void DirectX12::DisplayFrame()
{
    /// 描く状態から画面に映す状態に遷移
    /// (Direct2dで行われる)

    /// GPUとISに画面の交換を行うよう通知する
    swapChain_->Present(1, 0);

    // バックバッファのインデックスを取得
    backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

    /// Fenceの値を更新
    fenceValue_++;


    /// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
    commandQueue_->Signal(fence_.Get(), fenceValue_);

    /// GetCompletedValueの初期値はFence作成時に渡した初期値
    if (fence_->GetCompletedValue() < fenceValue_)
    {
        // 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
        fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
        // イベントを待つ
        WaitForSingleObject(fenceEvent_, INFINITE);
    }


    /// フレームレート固定
    pFramerate_->FixFramerate();


    /// 次のフレーム用のコマンドリストを準備
    hr_ = commandAllocator_->Reset();
    assert(SUCCEEDED(hr_));
    hr_ = commandList_->Reset(commandAllocator_.Get(), nullptr);
    assert(SUCCEEDED(hr_));
}

void DirectX12::CopyFromRTV(ID3D12GraphicsCommandList* commandList)
{
    #ifdef _DEBUG

    /// レンダーターゲットからコピー元状態にする
    swapChainResources_[backBufferIndex_].GetStateTracker().ChangeState(commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);

    gameScreenResource_.GetStateTracker().ChangeState(commandList, D3D12_RESOURCE_STATE_COPY_DEST);


    D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
    srcLocation.pResource        = swapChainResources_[backBufferIndex_].GetResource().Get();
    srcLocation.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    srcLocation.SubresourceIndex = 0; // コピーするサブリソースインデックス

    D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
    dstLocation.pResource        = gameScreenResource_.GetResource().Get();
    dstLocation.Type             = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dstLocation.SubresourceIndex = 0;

    /// コピー範囲
    D3D12_BOX srcBox = {};
    srcBox.left   = static_cast<UINT>(viewport_.TopLeftX);
    srcBox.top    = static_cast<UINT>(viewport_.TopLeftY);
    srcBox.right  = static_cast<UINT>(viewport_.Width + viewport_.TopLeftX);
    srcBox.bottom = static_cast<UINT>(viewport_.Height + viewport_.TopLeftY);
    srcBox.front  = 0;
    srcBox.back   = 1;

    commandList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, &srcBox);

    /// バリアを戻す
    swapChainResources_[backBufferIndex_].GetStateTracker().ChangeState(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

    gameScreenResource_.GetStateTracker().ChangeState(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    /// rtvのクリア
    commandList->ClearRenderTargetView(rtvHandles_[backBufferIndex_], &NimaEngine::Config::kEditorBGColor.x, 0, nullptr);

    #else

    commandList;

    #endif // _DEBUG
}

ID3D12GraphicsCommandList* DirectX12::GetCommandListLast() const
{
    if (!commandLists_.empty())
    {
        auto outerIt = std::prev(commandLists_.end());  // 外側の最後
        auto& lastList = outerIt->second;

        if (!lastList.empty())
        {
            auto innerIt = std::prev(lastList.end());   // 内側の最後
            auto& [key, cmd] = *innerIt;
            return cmd;
        }
    }
    return nullptr;
}

void DirectX12::AddCommandList(CommandListType type, ID3D12GraphicsCommandList* commandList, uint32_t order)
{
    uint32_t orderAssigned = order;
    uint32_t& orderNext = nextCLOrder_[CommandListType::ImGui];

    if (order == 0)
    {
        orderAssigned = orderNext;
    }

    // すでに同じorderが存在する場合は、orderをずらす
    while (commandLists_[type].find(orderAssigned) != commandLists_[type].end())
    {
        ++orderAssigned;
        if (order == 0)
        {
            orderNext = orderAssigned + 1;
        }
    }

    commandLists_[type].insert(std::make_pair(orderAssigned, commandList));
}

void DirectX12::RemoveCommandList(CommandListType type, ID3D12GraphicsCommandList* commandList)
{
    for (auto it = commandLists_[type].begin(); it != commandLists_[type].end(); ++it)
    {
        if (it->second == commandList)
        {
            commandLists_[type].erase(it);
            break;
        }
    }

}

DirectX12::~DirectX12()
{
    const UINT64 fenceSignalValue = ++fenceValue_;
    ID3D12Fence* fence = fence_.Get();

    commandQueue_->Signal(fence, fenceSignalValue);
    if (fence->GetCompletedValue() < fenceSignalValue)
    {
        fence->SetEventOnCompletion(fenceSignalValue, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
    }
}

void EngineFeature::SetDirectX12(DirectX12* _pDx12)
{
    pDx12_ = _pDx12;
}

DirectX12* EngineFeature::GetDirectX12()
{
    return pDx12_;
}
