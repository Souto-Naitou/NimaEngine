#pragma once

#include <Core/DirectX12/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>
#include <Effects/PostEffects/SeparatedGaussianFilter/SeparatedGaussianFilter.h>

struct alignas(16) LuminanceOutputOption
{
    float threshold = 0.5f; // Luminance threshold
    float padding[3]; // 16バイト境界に揃えるためのパディング
};

/// <ボックスフィルタ>
/// - ApplyメソッドとSettingメソッドはPostEffectクラスで実行する
class LuminanceOutput :
    public IPostEffect,
    public EngineFeature
{
public:
    void    Initialize() override;
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

    // =============================================
    // [Getters Begin]
    D3D12_GPU_DESCRIPTOR_HANDLE     GetOutputTextureHandle() const override;
    const std::string&              GetName() const override;
    LuminanceOutputOption&          GetOption();
    const LuminanceOutputOption&    GetOption() const;
    // [Getters End]
    // =============================================



private:
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;
    bool                                                isEnabled_              = false;
    const std::string                                   name_                   = "LuminanceOutput";

    // =============================================
    // [Input Begin]
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
    // [Input End]
    // =============================================

    // =============================================
    // [Output Begin]
    DX12Resource                                        outputTexture_          = {};
    D3D12_CPU_DESCRIPTOR_HANDLE                         rtvHandleCpu_           = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         rtvHandleGpu_           = {};
    uint32_t                                            rtvHeapIndex_           = 0;
    uint32_t                                            srvHeapIndex_           = 0;
    // [Output End]
    // =============================================

    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_       = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_        = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>         pso_                    = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_          = nullptr;
    const std::wstring                                  kVertexShaderPath       = L"EngineResources/Shaders/LuminanceOutput.VS.hlsl";
    const std::wstring                                  kPixelShaderPath        = L"EngineResources/Shaders/LuminanceOutput.PS.hlsl";

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D12Resource>              optionResource_         = nullptr;
    LuminanceOutputOption*                              pOption_                = nullptr;


    // Internal functions
    void    CreateRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
};