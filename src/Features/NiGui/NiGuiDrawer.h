#pragma once

#include <NiGui/Interface/NiGui_IDrawer.h>
#include <memory>
#include <Features/Sprite/Sprite.h>
#include <NiGui/Type/NiGui_Type_Core.h>

/// <summary>
/// NiGui描画クラス 具象クラス
/// </summary>
class NiGuiDrawer : public INiGuiDrawer
{
public:
    NiGuiDrawer() = default;
    ~NiGuiDrawer() = default;

    void Draw() override;
    void PlayAudio([[maybe_unused]]uint32_t _handle) override {};
    void PlayAudio(void* _audioHandler) override;

private:
    using SpriteList = std::vector<std::unique_ptr<Sprite>>;

    StringMap<SpriteList> sprites_;
    StringMap<uint32_t> textureCount_;
};