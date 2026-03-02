#pragma once

#include <Effects/PostEffects/PostEffectBase.h>
#include <Vector2.h>

struct alignas(16) ChromaticAberrationOption
{
    Vector2 direction;
    float   strength;
    float   padding0;
};

class ChromaticAberration : public PostEffectBase
{
public:
    void Initialize(const PostEffectInitParams& desc) override;
    void Setting() override;
    void DebugOverlay() override;

private:
    void CreateCBuffer(ID3D12Device* pDevice) override;
    void SetCBuffer(ID3D12GraphicsCommandList* pCommandList) override;
    void RegisterAdditionalRootParameter(RootParameters& rootParams) override;

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D12Resource>  pOptionResource_ = nullptr;
    ChromaticAberrationOption* pOption_ = nullptr;

    // 角度情報 (デバッグ用)
    float angleRadians_ = 0.0f;
};