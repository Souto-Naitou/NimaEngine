#include "Canvas.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Win32/WinSystem.h>
#include <Config/EngineSetting.h>
#include <imgui.h>


void Canvas::Initialize(const Canvas::Params& params)
{
    // パラメータチェック
    this->ParameterCheck(params);

    params_ = params;
    pDebugEntry_ = std::make_unique<DebugEntry<Canvas>>("Canvas", params.name, this);

    /// レンダーターゲット用リソースの作成
    auto tempResource = DX12Helper::CreateResourceForRenderTarget(
        params.pDx12->GetDevice(),
        WinSystem::clientWidth,
        WinSystem::clientHeight,
        NimaEngine::Config::kRenderTargetFormat,
        NimaEngine::Config::kEditorBGColor
    );

    DX12Resource::Params paramResource{};
    paramResource.resource = tempResource;
    paramResource.state = D3D12_RESOURCE_STATE_RENDER_TARGET;
    paramResource.format = NimaEngine::Config::kRenderTargetFormat;
    paramResource.name = params.name + "(canvas)";
    paramResource.pRTVCounter = params.pDx12->GetRTVHeapCounter();

    resource_.Initialize(paramResource);

    /// RTVの作成
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = NimaEngine::Config::kRenderTargetFormat;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    if (resource_.GetRTVHandle().ptr == 0)
    {
        const auto rtvHandleIndex = params.pDx12->GetRTVHeapCounter()->Allocate();
        const auto rtvCPUHandle = params.pDx12->GetRTVHeapCounter()->GetRTVHandle(rtvHandleIndex);
        resource_.SetRTV(rtvHandleIndex, rtvCPUHandle);
    }
    params.pDx12->GetDevice()->CreateRenderTargetView(
        resource_.GetResource().Get(),
        &rtvDesc,
        resource_.GetRTVHandle()
    );

    // ポストエフェクトに使用するSRVの生成
    resource_.CreateSRV();

    // ポストエフェクト実行クラスの生成
    pPostEffectExecuter_ = std::make_unique<PostEffectExecuter>();
    pPostEffectExecuter_->Initialize(params.pDx12, &resource_, false);

    // リサイズ時のコールバック登録
    params.pDx12->AddOnResizeBefore("PostEffect"+params.name, std::bind(&PostEffectExecuter::OnResizeBefore, pPostEffectExecuter_.get()));
    params.pDx12->AddOnResizeAfter("PostEffect"+params.name, std::bind(&PostEffectExecuter::OnResizeAfter, pPostEffectExecuter_.get()));
}

void Canvas::Finalize()
{
    params_.pDx12->DeleteOnResizeBefore("PostEffect" + params_.name);
    params_.pDx12->DeleteOnResizeAfter("PostEffect" + params_.name);

    #ifdef _DEBUG
    params_.pImGuiManager->RemoveImageResource(pPostEffectExecuter_->GetIntermediateResource());
    #endif // _DEBUG

    pPostEffectExecuter_->Finalize();
}

void Canvas::DrawObjects()
{
    auto cl = pPostEffectExecuter_->GetCommandList();
    auto clDx12 = params_.pDx12->GetCommandList();
    auto& rtvHandle = resource_.GetRTVHandle();
    DX12Helper::CommandListCommonSetting(params_.pDx12, cl, &rtvHandle);
    DX12Helper::CommandListCommonSetting(params_.pDx12, clDx12, &rtvHandle);

    if (params_.pCubemapSystem)
    {
        params_.pCubemapSystem->DrawSetting(clDx12);
        for (auto& skybox : skyboxes_)
        {
            skybox->Draw(clDx12);
        }
    }

    // 描画オブジェクトの描画
    for (auto& drawable : drawables_)
    {
        drawable->SetRTVHandle(rtvHandle);
        drawable->DrawCall();
    }
}

void Canvas::ApplyPostEffects()
{
    pPostEffectExecuter_->ApplyPostEffects();
}

void Canvas::Draw() const
{
    pPostEffectExecuter_->Draw();
}

void Canvas::ImGui()
{
#ifdef _DEBUG

    ImVec2 imageSize = {};
    float aspect = static_cast<float>(WinSystem::clientWidth) / static_cast<float>(WinSystem::clientHeight);
    auto cliSize = ImGui::GetContentRegionAvail();
    if (aspect >= 1.0f)
    {
        imageSize.x = cliSize.x;
        imageSize.y = cliSize.x / aspect;
    }
    else
    {
        imageSize.y = cliSize.y;
        imageSize.x = cliSize.y * aspect;
    }

    /// キャンバスの内容を表示 
    /// PostEffectExecuterの中間リソースを使用
    DX12Resource* resourceOutput = pPostEffectExecuter_->GetIntermediateResource();
    ImGui::Image((ImTextureID)resourceOutput->GetSRVHandleGPU().ptr, imageSize);
    params_.pImGuiManager->AddImageResource(resourceOutput);

    pPostEffectExecuter_->ImGui();

#endif // _DEBUG
}

void Canvas::ParameterCheck(const Canvas::Params& params) const
{
    if (params.pDx12 == nullptr)
    {
        throw std::runtime_error("Canvas::Initialize() : DirectX12 is nullptr.");
    }

    #ifdef _DEBUG
    if (params.pImGuiManager == nullptr)
    {
        throw std::runtime_error("Canvas::Initialize() : ImGuiManager is nullptr.");
    }
    #endif // _DEBUG
}

Canvas& Canvas::RegisterDrawable(Skybox* skybox)
{
    skyboxes_.push_back(skybox);
    return *this;
}

Canvas& Canvas::RegisterDrawable(DrawableBase* sprite)
{
    drawables_.push_back(sprite);
    return *this;
}

void Canvas::UnregisterDrawable(Skybox* skybox)
{
    skyboxes_.erase(std::remove(skyboxes_.begin(), skyboxes_.end(), skybox), skyboxes_.end());
}

void Canvas::UnregisterDrawable(DrawableBase* drawable)
{
    drawables_.remove(drawable);
}
