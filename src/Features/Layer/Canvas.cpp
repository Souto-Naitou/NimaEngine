#include "Canvas.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Win32/WinSystem.h>
#include <Config/EngineSetting.h>


void Canvas::Initialize(const CanvasInitParams& params)
{
    params_ = params;

    pDebugEntry_ = std::make_unique<DebugEntry<Canvas>>("Canvas", params.name, this);

    auto tempResource = DX12Helper::CreateResourceForRenderTarget(
        params.pDx12->GetDevice(),
        WinSystem::clientWidth,
        WinSystem::clientHeight,
        NimaEngine::Config::kRenderTargetFormat,
        NimaEngine::Config::kEditorBGColor
    );

    // リソースの初期化
    resource_.Initialize(
        tempResource, 
        D3D12_RESOURCE_STATE_RENDER_TARGET, 
        NimaEngine::Config::kRenderTargetFormat, 
        params.name + "(canvas)"
    );

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
    params_.pDx12->GetRTVHeapCounter()->Deallocate(resource_.GetRTVIndex());
    params_.pDx12->DeleteOnResizeBefore("PostEffect" + params_.name);
    params_.pDx12->DeleteOnResizeAfter("PostEffect" + params_.name);
    pPostEffectExecuter_->Finalize();
}

void Canvas::DrawObjects() const
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

    for (auto& sprite : sprites_)
    {
        sprite->SetRTVHandle(rtvHandle);
        sprite->Draw();
    }

    for (auto& object3d : objects3ds_)
    {
        object3d->SetRTVHandle(rtvHandle);
        object3d->Draw();
    }
}

void Canvas::ApplyPostEffects() const
{
    pPostEffectExecuter_->ApplyPostEffects();
}

void Canvas::Draw() const
{
    pPostEffectExecuter_->Draw();
}

void Canvas::ImGui()
{
    pPostEffectExecuter_->ImGui();
}

Canvas& Canvas::RegisterDrawable(Sprite* sprite)
{
    sprites_.push_back(sprite);
    return *this;
}

Canvas& Canvas::RegisterDrawable(Skybox* skybox)
{
    skyboxes_.push_back(skybox);
    return *this;
}

Canvas& Canvas::RegisterDrawable(Object3d* object3d)
{
    objects3ds_.push_back(object3d);
    return *this;
}

void Canvas::UnregisterDrawable(Sprite* sprite)
{
    sprites_.erase(std::remove(sprites_.begin(), sprites_.end(), sprite), sprites_.end());
}

void Canvas::UnregisterDrawable(Skybox* skybox)
{
    skyboxes_.erase(std::remove(skyboxes_.begin(), skyboxes_.end(), skybox), skyboxes_.end());
}

void Canvas::UnregisterDrawable(Object3d* object3d)
{
    objects3ds_.erase(std::remove(objects3ds_.begin(), objects3ds_.end(), object3d), objects3ds_.end());
}
