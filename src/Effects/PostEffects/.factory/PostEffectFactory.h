#pragma once

#include <Core/DirectX12/IPostEffect.h>
#include <Core/DirectX12/DirectX12.h>
#include <d3d12.h>
#include <memory>
#include <functional>
#include <unordered_map>

/// <summary>
/// ポストエフェクトクラス名列挙型
/// </summary>
enum class PostEffectClassName
{
    Grayscale,
    BoxFilter,
    RadialBlur,
    LuminanceOutput,
    PrewittOutline,
    GaussianFilter,
    SeparatedGaussianFilter,
    RandomFilter,
    GaussianBloom,
};

/// <summary>
/// ポストエフェクトファクトリー
/// </summary>
class PostEffectFactory
{
public:
    PostEffectFactory(DirectX12* pDx12, ID3D12GraphicsCommandList* pCommandList);
    std::unique_ptr<IPostEffect> CreatePostEffect(PostEffectClassName name);

private:
    void RegisterCreators();
    DirectX12* pDx12_;
    ID3D12GraphicsCommandList* pCommandList_;
    std::unordered_map<PostEffectClassName, std::function<std::unique_ptr<IPostEffect>()>> creators_;
};