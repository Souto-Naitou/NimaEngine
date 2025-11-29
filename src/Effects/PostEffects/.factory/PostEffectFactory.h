#pragma once

#include <Core/DirectX12/IPostEffect.h>
#include <Core/DirectX12/DirectX12.h>
#include <d3d12.h>
#include <memory>
#include <functional>
#include <unordered_map>
#include <map>
#include <string>

/// <summary>
/// ポストエフェクトクラス名列挙型
/// </summary>
enum class PostEffectClassName
{
    None,
    Grayscale,
    BoxFilter,
    RadialBlur,
    LuminanceOutput,
    PrewittOutline,
    DepthBasedOutline,
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
    static const std::map<PostEffectClassName, std::string> nameMap_;

    PostEffectFactory(DirectX12* pDx12, ID3D12GraphicsCommandList* pCommandList);
    std::unique_ptr<IPostEffect> CreatePostEffect(PostEffectClassName name);

private:
    void RegisterCreators();
    DirectX12* pDx12_;
    ID3D12GraphicsCommandList* pCommandList_;
    std::unordered_map<PostEffectClassName, std::function<std::unique_ptr<IPostEffect>()>> creators_;
};