#include "Canvas.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Core/Win32/WinSystem.h>
#include <Config/EngineSetting.h>


void Canvas::Initialize(const CanvasInitParams& params)
{
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

    // ポストエフェクト実行クラスの生成
    pPostEffectExecuter_ = std::make_unique<PostEffectExecuter>();
    pPostEffectExecuter_->Initialize(params.pDx12);
}

void Canvas::Draw() const
{
    // 登録されたスプライトを描画
    for (auto& sprite : sprites_)
    {
        sprite->Draw();
    }

    // 描画用コマンドリストの取得
    pPostEffectExecuter_->ApplyPostEffects();
}

void Canvas::ImGui()
{


}

Canvas& Canvas::RegisterDrawable(Sprite* sprite)
{
    sprites_.push_back(sprite);
    return *this;
}