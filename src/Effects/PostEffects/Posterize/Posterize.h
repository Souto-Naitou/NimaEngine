#pragma once

#include <Effects/PostEffects/PostEffectBase.h>
#include <wrl/client.h>

struct PosterizeOption
{
    float level;
    float padding0[3];
};

class Posterize : public PostEffectBase
{
public:
    void Initialize(const PostEffectInitParams& params) override;
    void DebugOverlay() override;
    void CreateCBuffer(ID3D12Device*) override;
    void SetCBuffer(ID3D12GraphicsCommandList*) override;
    void RegisterAdditionalRootParameter(RootParameters&) override;

private:
    PosterizeOption* pOption_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> pOptionResource_ = nullptr;
};