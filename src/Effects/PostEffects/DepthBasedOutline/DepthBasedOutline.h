#pragma once

#include <Core/DirectX12/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Matrix4x4.h>
#include <Core/DirectX12/PipelineStateObject/PipelineStateObject.h>

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

    void    Enable(bool _flag) override;
    bool    Enabled() const override;

    void    Apply() override;
    void    Setting() override;
    void    OnResizeBefore() override;
    void    OnResizedBuffers() override;
    void    ToShaderResourceState() override;
    void    DebugOverlay() override;

    // Setters
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle) override;

    // Getters
    D3D12_GPU_DESCRIPTOR_HANDLE         GetOutputTextureHandle() const override;
    const std::string&                  GetName() const override;
    DepthBasedOutlineOption&            GetOption();
    const DepthBasedOutlineOption&      GetOption() const;
    DepthBasedOutlineMaterial&          GetMaterial();
    const DepthBasedOutlineMaterial&    GetMaterial() const;

private:
    DirectX12*                                          pDx12_                  = nullptr;
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;

    bool                                                isEnabled_              = false;
    const std::string                                   name_                   = "DepthBasedOutline";
    DX12Resource                                        renderTexture_          = {};
    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_       = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_        = nullptr;
    PipelineStateObject                                 pso_                    = {};
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_          = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE                         rtvHandleCpu_           = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         rtvHandleGpu_           = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         depthGpuHandle_         = {};
    uint32_t                                            rtvHeapIndex_           = 0;
    uint32_t                                            srvHeapIndex_           = 0;
    uint32_t                                            srvIndexDepth_          = 0;
    const std::wstring                                  kVertexShaderPath       = L"EngineResources/Shaders/DepthBasedOutline.VS.hlsl";
    const std::wstring                                  kPixelShaderPath        = L"EngineResources/Shaders/DepthBasedOutline.PS.hlsl";

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D12Resource>              optionResource_         = nullptr;
    DepthBasedOutlineOption*                            pOption_                = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>              materialResource_       = nullptr;
    DepthBasedOutlineMaterial*                          pMaterial_              = nullptr;

    // Internal functions
    void    CreateRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
    void    CreateSRV();
};