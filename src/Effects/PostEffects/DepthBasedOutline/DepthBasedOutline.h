#pragma once

#include <Effects/PostEffects/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Matrix4x4.h>
#include <Core/DirectX12/PipelineStateObject/PSOBuilder.h>
#include <Core/DirectX12/RootSignature/RootSignatureCache.h>
#include <Core/DirectX12/PipelineStateObject/PSOCache.h>

struct alignas(16) DepthBasedOutlineOption
{
    float weightMultiply = 1.0f;
    float padding[3] = {};
};

struct alignas(16) DepthBasedOutlineMaterial
{
    Matrix4x4 projectionInverse;
};

/// 深度ベースアウトライン
class DepthBasedOutline : public IPostEffect
{
public:
    void    Initialize(const PostEffectInitParams& desc) override;
    void    Finalize() override;

    void    Enable(bool flag) override;
    bool    Enabled() const override;

    void    Apply() override;
    void    Setting() override;
    void    OnResizeBefore() override;
    void    OnResizeAfter() override;
    void    ToShaderResourceState() override;
    void    DebugOverlay() override;

    // Setters
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) override;

    // Getters
    D3D12_GPU_DESCRIPTOR_HANDLE         GetOutputTextureHandle() const override;
    const std::string&                  GetName() const override;
    DX12Resource*                     GetOutputResource() const override;
    DepthBasedOutlineOption&            GetOption();
    const DepthBasedOutlineOption&      GetOption() const;
    DepthBasedOutlineMaterial&          GetMaterial();
    const DepthBasedOutlineMaterial&    GetMaterial() const;

private:
    const PSOID                                         kPSOId_                 = "DepthBasedOutline";
    const RootSignatureID                               kRootSignatureId_       = "DepthBasedOutline";
    const std::string                                   name_                   = "DepthBasedOutline";
    const std::wstring                                  kVertexShaderPath       = L"EngineResources/Shaders/DepthBasedOutline.VS.hlsl";
    const std::wstring                                  kPixelShaderPath        = L"EngineResources/Shaders/DepthBasedOutline.PS.hlsl";

    DirectX12*                                          pDx12_                  = nullptr;
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;

    bool                                                isEnabled_              = false;
    DX12Resource                                        renderTexture_          = {};
    ID3D12PipelineState*                                pso_                    = nullptr;
    ID3D12RootSignature*                                rootSignature_          = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         depthGpuHandle_         = {};
    uint32_t                                            srvIndexDepth_          = 0;

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D12Resource>              optionResource_         = nullptr;
    DepthBasedOutlineOption*                            pOption_                = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>              materialResource_       = nullptr;
    DepthBasedOutlineMaterial*                          pMaterial_              = nullptr;

    // Internal functions
    void    RegisterRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
    void    CreateSRV();
};