#pragma once

#include <Core/DirectX12/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>
#include <Vector4.h>
#include <Vector3.h>
#include <Vector2.h>

struct alignas(16) RadialBlurOption
{
    Vector2 center = Vector2(0.5f, 0.5f); // 中心位置 (0.0f ~ 1.0f)
    int samples = 16; // サンプル数
    float blurWidth = 0.01f; // 幅 (0.0f ~ 1.0f)
};

/// <ラジアルブラー>
/// - ApplyメソッドとSettingメソッドはPostEffectクラスで実行する
class RadialBlur : 
    public IPostEffect,
    public EngineFeature
{
public:
    void    Initialize() override;
    void    Finalize() override;

    void    Enable(bool _flag) override;
    bool    Enabled() const override;

    // Setter (Additional)
    void    SetCenter(float _center) { pOption_->center = _center; }
    void    SetSamples(int _sample) { pOption_->samples = _sample; }
    void    SetBlurWidth(float _width) { pOption_->blurWidth = _width; }

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
    RadialBlurOption&               GetOption();
    const RadialBlurOption&         GetOption() const;
    // [Getters End]
    // =============================================


private:
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;

    bool                                                isEnabled_              = false;
    const std::string                                   name_                   = "RadialBlur";
    DX12Resource                                        renderTexture_          = {};
    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_       = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_        = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>         pso_                    = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_          = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE                         rtvHandleCpu_           = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         rtvHandleGpu_           = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
    uint32_t                                            rtvHeapIndex_           = 0;
    uint32_t                                            srvHeapIndex_           = 0;
    const std::wstring                                  kVertexShaderPath       = L"EngineResources/Shaders/RadialBlur.VS.hlsl";
    const std::wstring                                  kPixelShaderPath        = L"EngineResources/Shaders/RadialBlur.PS.hlsl";

    // Constant buffer view
    Microsoft::WRL::ComPtr<ID3D12Resource>              optionResource_     = nullptr;
    RadialBlurOption*                                   pOption_            = nullptr;


    // Internal functions
    void    CreateRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
};