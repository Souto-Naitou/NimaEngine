#include "PostEffectFactory.h"
#include <Effects/PostEffects/Grayscale/Grayscale.h>
#include <Effects/PostEffects/BoxFilter/BoxFilter.h>
#include <Effects/PostEffects/RadialBlur/RadialBlur.h>
#include <Effects/PostEffects/LuminanceOutput/LuminanceOutput.h>
#include <Effects/PostEffects/PrewittOutline/PrewittOutline.h>
#include <Effects/PostEffects/GaussianFilter/GaussianFilter.h>
#include <Effects/PostEffects/SeparatedGaussianFilter/SeparatedGaussianFilter.h>
#include <Effects/PostEffects/RandomFilter/RandomFilter.h>
#include <Effects/PostEffects/GaussianBloom/GaussianBloom.h>

PostEffectFactory::PostEffectFactory(DirectX12* pDx12, ID3D12GraphicsCommandList* pCommandList)
{
    pDx12_ = pDx12;
    pCommandList_ = pCommandList;
    this->RegisterCreators();
}

std::unique_ptr<IPostEffect> PostEffectFactory::CreatePostEffect(PostEffectClassName name)
{
    return creators_[name]();
}

void PostEffectFactory::RegisterCreators()
{
    creators_[PostEffectClassName::Grayscale] = [this]() {return std::make_unique<Grayscale>();};
    creators_[PostEffectClassName::BoxFilter] = [this]() { return std::make_unique<BoxFilter>(); };
    creators_[PostEffectClassName::RadialBlur] = [this]() { return std::make_unique<RadialBlur>(); };
    creators_[PostEffectClassName::LuminanceOutput] = [this]() { return std::make_unique<LuminanceOutput>(); };
    creators_[PostEffectClassName::PrewittOutline] = [this]() { return std::make_unique<PrewittOutline>(); };
    creators_[PostEffectClassName::GaussianFilter] = [this]() { return std::make_unique<GaussianFilter>(); };
    creators_[PostEffectClassName::SeparatedGaussianFilter] = [this]() { return std::make_unique<SeparatedGaussianFilter>(); };
    creators_[PostEffectClassName::RandomFilter] = [this]() { return std::make_unique<RandomFilter>(); };
    creators_[PostEffectClassName::GaussianBloom] = [this]() { return std::make_unique<GaussianBloom>(); };
}
