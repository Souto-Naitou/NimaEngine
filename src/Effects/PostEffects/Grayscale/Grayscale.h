#pragma once

#include <Effects/PostEffects/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/DX12Resource/DX12Resource.h>

struct alignas(16) GrayscaleOption
{
    float power = 1.0f; // Luminance threshold
    float padding[3]; // 16バイト境界に揃えるためのパディング
};

/// グレースケール
class Grayscale : public IPostEffect
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
    D3D12_GPU_DESCRIPTOR_HANDLE     GetOutputTextureHandle() const override;
    const std::string&              GetName() const override;
    /// <summary>適用後のリソースを取得します。</summary>
    DX12Resource*      GetOutputResource() const override;
    /// <summary>オプションへの参照を取得します。</summary>
    GrayscaleOption&   GetOption() { return *pOption_; }
    const GrayscaleOption& GetOption() const { return *pOption_; }

private:
    DirectX12*                                          pDx12_                  = nullptr;
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;

    bool                                                isEnabled_              = false;
    const std::string                                   name_                   = "Grayscale";
    GrayscaleOption                                     option_                 = {};
    DX12Resource                                        renderTexture_          = {};
    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_       = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_        = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>         pso_                    = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_          = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
    const std::wstring                                  kVertexShaderPath       = L"EngineResources/Shaders/Grayscale.VS.hlsl";
    const std::wstring                                  kPixelShaderPath        = L"EngineResources/Shaders/Grayscale.PS.hlsl";
    GrayscaleOption*                                    pOption_                = {};
    Microsoft::WRL::ComPtr<ID3D12Resource>              optionResource_         = nullptr;


    // Internal functions
    void    CreateRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResorceCBuffer();
};