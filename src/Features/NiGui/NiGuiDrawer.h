#pragma once

#include <NiGui/Interface/NiGui_IDrawer.h>
#include <memory>
#include <drawable/sprite/Sprite.h>
#include <NiGui/Type/NiGui_Type_Core.h>
#include <Features/Layer/OrderedCanvasLayer.h>
#include <Features/Layer/Canvas.h>

/// <summary>
/// NiGui描画クラス 実装クラス
/// </summary>
class NiGuiDrawer : public INiGuiDrawer
{
public:
    NiGuiDrawer() = default;
    ~NiGuiDrawer() = default;

    void CreateAndRegisterCanvas(OrderedCanvasLayer* pLayer, const Canvas::Params& canvasParams);
    void UnregisterCanvas();
    void Draw() override;
    void PlayAudio([[maybe_unused]]uint32_t handle) override {};
    void PlayAudio(void* audioHandler) override;

private:
    using SpriteList = std::vector<std::unique_ptr<Sprite>>;

    std::unique_ptr<Canvas> pCanvas_ = nullptr;
    StringMap<SpriteList> sprites_;
    StringMap<uint32_t> textureCount_;
    OrderedCanvasLayer* pLayer_ = nullptr;
};