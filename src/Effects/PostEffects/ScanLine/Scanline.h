#pragma once

#include <Effects/PostEffects/IPostEffect.h>
#include "Core/DirectX12/PipelineStateObject/PSOCache.h"
#include "Core/DirectX12/RootSignature/RootSignatureCache.h"
#include <Vector2.h>
#include <Vector4.h>

class Scanline : public IPostEffect
{
public:
    struct ScanlineOption
    {
        float division;
        float speed;
        float opacity;
        float isOverall;
        Vector4 color0;
        Vector4 color1;
    };

    void Initialize(const PostEffectInitParams& desc) override;
    void Finalize() override;

    inline void Enable(bool flag) override { isEnabled_ = flag; }
    inline bool Enabled() const override { return isEnabled_; }
    inline void SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) override { inputGpuHandle_ = handle; }
    inline D3D12_GPU_DESCRIPTOR_HANDLE GetOutputTextureHandle() const override { return pRenderTexture_->GetSRVHandleGPU(); }
    inline const std::string& GetName() const override { return kName_; }
    inline DX12Resource* GetOutputResource() const override { return pRenderTexture_.get(); }
    void Setting() override;
    void Apply() override;
    void ToShaderResourceState() override;
    void OnResizeBefore() override;
    void OnResizeAfter() override;
    void DebugOverlay() override;

    ScanlineOption& GetOption() { return *pOption_; }
    const ScanlineOption& GetOption() const { return *pOption_; }

private:
    const PSOID                             kPSOId_             = "Scanline";
    const RootSignatureID                   kRootSignatureId_   = "Scanline";
    const std::string                       kName_              = "Scanline";
    const std::wstring                      kVertexShaderPath_  = L"EngineResources/Shaders/PostEffectCommon.VS.hlsl";
    const std::wstring                      kPixelShaderPath_   = L"EngineResources/Shaders/Scanline.PS.hlsl";

    DirectX12*                              pDx12_              = nullptr;
    ID3D12Device*                           pDevice_            = nullptr;
    ID3D12GraphicsCommandList*              pCommandList_       = nullptr;

    bool                                    isEnabled_          = false;
    std::unique_ptr<DX12Resource>           pRenderTexture_     = {};
    ID3D12PipelineState*                    pPSO_               = nullptr;
    ID3D12RootSignature*                    pRootSignature_     = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE             inputGpuHandle_     = {};

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D12Resource>  pOptionResource_    = nullptr;
    ScanlineOption*                         pOption_            = nullptr;

    void    RegisterRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
};