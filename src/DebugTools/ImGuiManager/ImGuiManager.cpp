#include "ImGuiManager.h"

#ifdef _DEBUG

#include <Core/Win32/WinSystem.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/SRVManager.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <config/EngineSetting.h>

#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#include <Core/DirectX12/Helper/DX12Helper.h>

void ImGuiManager::Initialize()
{
    InitializeStyleNameArray();

    SRVManager* srvManager = SRVManager::GetInstance();
    srvIndex_ = srvManager->Allocate();

    WinSystem* pWin32App = WinSystem::GetInstance();
    ID3D12Device* device = pDx12_->GetDevice();
    const DXGI_SWAP_CHAIN_DESC1& swapChainDesc = pDx12_->GetSwapChainDesc();
    srvDescHeap_ = srvManager->GetDescriptorHeap();
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = srvManager->GetCPUDescriptorHandle(srvIndex_);
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = srvManager->GetGPUDescriptorHandle(srvIndex_);

    // コマンドリスト周りの初期化
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    pDx12_->AddCommandList(DirectX12::CommandListType::ImGui, commandList_.Get());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplDX12_Init(
        device,
        swapChainDesc.BufferCount,
        NimaEngine::Config::kRenderTargetFormat,
        srvDescHeap_,
        cpuHandle,
        gpuHandle
    );
    ImGui_ImplWin32_Init(pWin32App->GetHwnd());

    io_ = &ImGui::GetIO();

    DebugManager::GetInstance()->SetComponent("Core", "ImGui", std::bind(&ImGuiManager::ImGui, this));
}

void ImGuiManager::Finalize()
{
    DebugManager::GetInstance()->DeleteComponent("Core", "ImGui");
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiManager::OnResizedBuffers()
{
    io_->DisplaySize.x = static_cast<float>(WinSystem::clientWidth);
    io_->DisplaySize.y = static_cast<float>(WinSystem::clientHeight);

    float scaleX = static_cast<float>(WinSystem::clientWidth) / static_cast<float>(WinSystem::preClientWidth);
    float scaleY = static_cast<float>(WinSystem::clientHeight) / static_cast<float>(WinSystem::preClientHeight);

    io_->DisplayFramebufferScale = ImVec2(scaleX, scaleY);

    auto* drawData = ImGui::GetDrawData();
    if (drawData)
    {
        drawData->DisplayPos = ImVec2(0.0f, 0.0f);
        drawData->DisplaySize = ImVec2(io_->DisplaySize.x, io_->DisplaySize.y);
    }
}

void ImGuiManager::AddImageResource(DX12Resource* resource)
{
    imageResources_.emplace_back(std::make_pair(resource, D3D12_RESOURCE_STATE_COMMON));
}

void ImGuiManager::RemoveImageResource(DX12Resource* resource)
{
    imageResources_.erase(
        std::remove_if(
            imageResources_.begin(),
            imageResources_.end(),
            [resource](const auto& pair) { return pair.first == resource; }
        ),
        imageResources_.end()
    );
}

void ImGuiManager::EnableDocking()
{
    #ifdef _DEBUG
    io_->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    #endif
}

void ImGuiManager::EnableMultiViewport()
{
    #ifdef _DEBUG
    io_->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    #endif
}

void ImGuiManager::BeginFrame()
{
    if (!isChangedFont_)
    {
        DebugManager* debugManager = DebugManager::GetInstance();
        debugManager->ChangeFont();

        isChangedFont_ = true;
    }

    imageResources_.clear();

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::Render()
{
    ImGui::EndFrame();
    ImGui::Render();

    /// 使用するリソースの状態を遷移
    for (auto& resource : imageResources_)
    {
        auto& stateTracker = resource.first->GetStateTracker();
        resource.second = stateTracker.ChangeState(commandList_.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    uint32_t indexBackbuffer = pDx12_->GetBackBufferIndex();
    auto rtvHandleSwapChain_ = pDx12_->GetRTVHandle()[indexBackbuffer];
    DX12Helper::CommandListCommonSetting(pDx12_, commandList_.Get(), &rtvHandleSwapChain_);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList_.Get());

    /// 使用したリソースの状態を元に戻す
    for (auto& resource : imageResources_)
    {
        auto& stateTracker = resource.first->GetStateTracker();
        resource.second = stateTracker.ChangeState(commandList_.Get(), resource.second);
    }
}

void ImGuiManager::PostDraw()
{
    commandAllocator_->Reset();
    commandList_->Reset(commandAllocator_.Get(), nullptr);
}

void ImGuiManager::ImGui()
{
    bool isChange = false;
    if (ImGui::BeginCombo("Style", styleNameArray_[idx_currentStyle_].c_str()))
    {
        for (size_t i = 0; i < styleNameArray_.size(); ++i)
        {
            const bool isSelected = i == idx_currentStyle_;
            if (ImGui::Selectable(styleNameArray_[i].c_str(), isSelected))
            {
                idx_currentStyle_ = i;
                isChange = true;
            }
        }
        ImGui::EndCombo();
    }

    if (isChange)
    {
        switch (idx_currentStyle_)
        {
        case 0:
            this->StyleOriginal();
            break;
        case 1:
            this->StylePhotoshop();
            break;
        case 2:
            this->StyleMaterialFlat();
            break;
        case 3:
            this->StyleFutureDark();
            break;
        case 4:
            this->StyleComfortableDarkCyan();
            break;
        default:
            break;
        }
    }
}

void ImGuiManager::InitializeStyleNameArray()
{
    styleNameArray_.push_back("Original");
    styleNameArray_.push_back("Photoshop");
    styleNameArray_.push_back("Material Flat");
    styleNameArray_.push_back("Future Dark");
    styleNameArray_.push_back("Comfortable Dark Cyan");
}

#endif // _DEBUG