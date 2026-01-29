#include "NiGuiDrawer.h"

#include <Features/Audio/Audio.h>
#include <Features/Layer/CanvasScope.h>

void NiGuiDrawer::CreateAndRegisterCanvas(OrderedCanvasLayer* pLayer, const Canvas::Params& canvasParams)
{
    pCanvas_ = std::make_unique<Canvas>();
    pCanvas_->Initialize(canvasParams);
    pLayer_ = pLayer;
    pLayer_->AddCanvasTop(pCanvas_.get());
}

void NiGuiDrawer::UnregisterCanvas()
{
    pCanvas_->Finalize();
    pLayer_->RemoveCanvas(pCanvas_.get());
}

void NiGuiDrawer::Draw()
{
    CanvasScope canvasScope(pCanvas_.get());
    textureCount_.clear();

    for (auto& data : drawDataZOrdered_)
    {
        const auto& textureName = data->textureName;
        auto& currentIndex = textureCount_[textureName];
        auto& spriteList = sprites_[textureName];
        Sprite* sprite = nullptr;

        // 足りなくなったら追加
        if (spriteList.size() == textureCount_[textureName])
        {
            spriteList.push_back(std::make_unique<Sprite>());
            sprite = spriteList.back().get();
            sprite->Initialize(textureName);
        }
        else
        {
            sprite = sprites_[data->textureName][currentIndex].get();
        }

        sprite->SetPosition({ data->leftTop.x, data->leftTop.y });
        sprite->SetSize({ data->size.x, data->size.y });
        sprite->SetColor({ data->color.x, data->color.y, data->color.z, data->color.w });

        sprite->Update();
        sprite->Draw1F();

        currentIndex++;
    }
}

void NiGuiDrawer::PlayAudio(void* audioHandler)
{
    Audio* audio = static_cast<Audio*>(audioHandler);
    audio->Play();
}
