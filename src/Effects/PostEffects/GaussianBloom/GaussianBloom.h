#pragma once

#include <Core/DirectX12/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>
#include <Effects/PostEffects/SeparatedGaussianFilter/SeparatedGaussianFilter.h>
#include <Effects/PostEffects/LuminanceOutput/LuminanceOutput.h>

struct GaussianBloomOption
{
    float bloomIntensity = 1.0f; // ブルームの強度
};

// 手順
// 1. Initializeメソッドでリソースを生成
// 2. Enableメソッドで有効化フラグを設定
// 3. SetInputTextureHandleメソッドで入力テクスチャのハンドルを設定
// 4. Settingメソッドでシェーダーリソースやレンダーターゲットの設定を行う
// 5. Applyメソッドでポストエフェクトを適用
//   a. ルミナンスを取得して対象ピクセルを保存
//   b. ルミナンスを元にガウスフィルタを適用
//   c. 結果を出力リソースに書き込む
//   input -> luminance -> gaussian filter -> output

/// ガウシアンブルーム
class GaussianBloom : public IPostEffect
{
public:
    void    Initialize(const PostEffectInitDesc& desc)                override;
    void    Finalize()                  override;
    void    Enable(bool _flag)          override;
    bool    Enabled() const             override;
    void    Apply()                     override;
    void    Setting()                   override;
    void    OnResizeBefore()            override;
    void    OnResizedBuffers()          override;
    void    ToShaderResourceState()     override;
    void    DebugOverlay()              override;

    // Setters
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle) override;

    // =============================================
    // [Getter Begin]
    D3D12_GPU_DESCRIPTOR_HANDLE             GetOutputTextureHandle()        const override;
    const std::string&                      GetName()                       const override;
    const GaussianBloomOption&              GetOption()                     const;
    const SeparatedGaussianFilter*          GetSeparatedGaussianFilter()    const;
    const LuminanceOutput*                  GetLuminanceOutputFilter()      const;
    GaussianBloomOption&                    GetOption();
    SeparatedGaussianFilter*                GetSeparatedGaussianFilter();
    LuminanceOutput*                        GetLuminanceOutputFilter();
    // [Getter End]
    // =============================================


private:
    DirectX12*                                          pDx12_                  = nullptr;
    ID3D12Device*                                       device_                     = nullptr;
    ID3D12GraphicsCommandList*                          commandList_                = nullptr;
    bool                                                isEnabled_                  = false;
    const std::string                                   name_                       = "GaussianBloom";

    // =============================================
    // [Input Data Begin]
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_             = {};
    // [Input Data End]
    // =============================================

    // =============================================
    // [Luminance output Begin]
    std::unique_ptr<LuminanceOutput>                    pLuminanceOutput_           = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandleLuminance_    = {};
    // [Luminance output End]
    // =============================================

    // =============================================
    // [Gaussian Filter Effect Begin]
    std::unique_ptr<SeparatedGaussianFilter>            pSeparatedGaussianFilter_   = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandleGaussian_     = {};
    // [Gaussian Filter Effect End]
    // =============================================

    // =============================================
    // [Final Output Begin]
    DX12Resource                                        outputTexture_              = {};
    D3D12_CPU_DESCRIPTOR_HANDLE                         rtvHandleCpu_               = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         rtvHandleGpu_               = {};
    uint32_t                                            rtvHeapIndex_               = 0;
    uint32_t                                            srvHeapIndex_               = 0;
    // [Final Output End]
    // =============================================

    // =============================================
    // [ConstantBuffers Begin]
    Microsoft::WRL::ComPtr<ID3D12Resource>              cbOptionResorce_            = nullptr;
    GaussianBloomOption*                                cbOptionData_               = nullptr;
    // [ConstantBuffers End]
    // =============================================

    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_           = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_            = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>         pso_                        = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_              = nullptr;
    const std::wstring                                  kVertexShaderPath           = L"EngineResources/Shaders/GaussianBloom.VS.hlsl";
    const std::wstring                                  kPixelShaderPath            = L"EngineResources/Shaders/GaussianBloom.PS.hlsl";

    // Internal functions
    void    CreateRootSignature();
    void    CreatePipelineStateObject();
    void    PreDrawSetting(D3D12_GPU_DESCRIPTOR_HANDLE _inputGpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE _outputCpuHandle);
    void    InitializeLuminanceOutputFilter();
    void    InitializeSeparatedGaussianFilter();
    void    CreateResourceCBuffer();
    D3D12_GPU_DESCRIPTOR_HANDLE     ApplyFilter(D3D12_GPU_DESCRIPTOR_HANDLE _inputGpuHandle, IPostEffect* _pEffect);
};