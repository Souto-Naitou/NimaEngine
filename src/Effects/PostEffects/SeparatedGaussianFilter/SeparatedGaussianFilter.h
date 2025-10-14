#pragma once

#include <Core/DirectX12/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>

struct alignas(16) SeparatedGaussianFilterOption
{
    int kernelSize = 5;
    float padding[3];
    struct alignas(16) Weight
    {
        float value;
        float padding[3]; // 16バイト境界に揃えるためのパディング
    } weights[31];
};

struct alignas(16) SeparatedGaussianFilterExecInfo
{
    int direction[2];
    float padding[2]; // 16バイト境界に揃えるためのパディング
};

/// <ボックスフィルタ>
/// - ApplyメソッドとSettingメソッドはPostEffectクラスで実行する
class SeparatedGaussianFilter :
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
    void    SetSigma(float _sigma);

    // Getters
    D3D12_GPU_DESCRIPTOR_HANDLE             GetOutputTextureHandle() const override;
    const std::string&                      GetName() const override;
    SeparatedGaussianFilterOption&          GetOption();
    const SeparatedGaussianFilterOption&    GetOption() const;

private:
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;

    bool                                                isEnabled_              = false;
    const std::string                                   name_                   = "SeparatedGaussianFilter";
    DX12Resource                                        horizontalGaussTexture_ = {};
    DX12Resource                                        renderTexture_          = {};
    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_       = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_        = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>         pso_                    = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_          = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE                         rtvHandleCpu_           = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         rtvHandleGpu_           = {};
    D3D12_CPU_DESCRIPTOR_HANDLE                         horizontalHandleCpu_    = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         horizontalHandleGpu_    = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
    uint32_t                                            rtvHeapIndex_           = 0;
    uint32_t                                            horizontalHeapIndex_    = 0;
    uint32_t                                            srvHeapIndex_           = 0;
    uint32_t                                            horizontalSrvHeapIndex_ = 0;
    const std::wstring                                  kVertexShaderPath       = L"EngineResources/Shaders/SeparatedGaussianFilter.VS.hlsl";
    const std::wstring                                  kPixelShaderPath        = L"EngineResources/Shaders/SeparatedGaussianFilter.PS.hlsl";
    constexpr static const char*                        kNameHorizontal         = "RT_SeparatedGaussianFilter(Horizontal)";
    constexpr static const char*                        kNameVertical           = "RT_SeparatedGaussianFilter(Vertical)";

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D12Resource>              optionResource_             = nullptr;
    SeparatedGaussianFilterOption*                      pOption_                    = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>              execInfoResourceVertical_   = {};
    Microsoft::WRL::ComPtr<ID3D12Resource>              execInfoResourceHorizontal_ = {};
    SeparatedGaussianFilterExecInfo*                    pExecInfoVertical_          = {};
    SeparatedGaussianFilterExecInfo*                    pExecInfoHorizontal_        = {};

    // Parameters
    float sigma_ = 1.0f; // シグマの初期値

    // Internal functions
    void    CreateRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
    void    PreDrawSetting(D3D12_GPU_DESCRIPTOR_HANDLE _inputGpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE _outputCpuHandle, ID3D12Resource* _execInfoResource);
    void    CreateKernel();
};