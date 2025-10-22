#pragma once

#include <Core/DirectX12/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>
#include <Core/DirectX12/PipelineStateObject/PipelineStateObject.h>
#include <Vector4.h>
#include <Vector3.h>

/// ランダムフィルタ
class RandomFilter : public IPostEffect
{
public:
    struct RandomFilterOption
    {
        float seed = 0.0f;
        float opacity = 1.0f;
    };

public:
    void    Initialize(const PostEffectInitDesc& desc) override;
    void    Finalize() override;

    void    Enable(bool _flag) override;
    bool    Enabled() const override;

    // オプションの設定
    void    SetOpacity(float _opacity);
    void    SetSeed(float _seed);

    void    Apply() override;
    void    Setting() override;
    void    OnResizeBefore() override;
    void    OnResizedBuffers() override;
    void    ToShaderResourceState() override;
    void    DebugOverlay() override;

    // Setters
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle) override;

    // Getters
    D3D12_GPU_DESCRIPTOR_HANDLE     GetOutputTextureHandle() const override;
    const std::string&              GetName() const override;
    RandomFilterOption&             GetOption();
    const RandomFilterOption&       GetOption() const;

private:
    DirectX12*                                          pDx12_                  = nullptr;
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;

    bool                                                isEnabled_              = false;
    const std::string                                   name_                   = "RandomFilter";
    DX12Resource                                        renderTexture_          = {};
    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_       = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_        = nullptr;
    PipelineStateObject                                 pso_                    = {};
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_          = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
    const std::wstring                                  kVertexShaderPath       = L"EngineResources/Shaders/RandomFilter.VS.hlsl";
    const std::wstring                                  kPixelShaderPath        = L"EngineResources/Shaders/RandomFilter.PS.hlsl";

    // Constant buffer view
    Microsoft::WRL::ComPtr<ID3D12Resource>              optionResource_     = nullptr;
    RandomFilterOption*                                 pOption_            = nullptr;


    // Internal functions
    void    CreateRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
};