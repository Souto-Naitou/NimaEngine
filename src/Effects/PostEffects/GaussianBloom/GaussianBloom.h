#pragma once

#include <Effects/PostEffects/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>
#include <Core/DirectX12/RootSignature/RootSignatureCache.h>
#include <Core/DirectX12/PipelineStateObject/PSOCache.h>
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
    /// <summary>
    /// エフェクトを初期化し、必要なリソースを確保します。
    /// </summary>
    /// <param name="desc">DX12 まわりの初期化情報。</param>
    void    Initialize(const PostEffectInitParams& desc)                override;
    
    /// <summary>
    /// リソースを解放します。
    /// </summary>
    void    Finalize()                  override;
    
    /// <summary>
    /// エフェクトの有効/無効を設定します。
    /// </summary>
    /// <param name="flag">true で有効。</param>
    void    Enable(bool flag)          override;
    
    /// <summary>
    /// エフェクトが有効かどうかを返します。
    /// </summary>
    bool    Enabled() const             override;
    
    /// <summary>
    /// エフェクトを適用します。
    /// </summary>
    void    Apply()                     override;
    
    /// <summary>
    /// 描画のための設定を行います。
    /// </summary>
    void    Setting()                   override;
    
    /// <summary>
    /// レンダーターゲットをシェーダーリソース状態へ遷移させます。
    /// </summary>
    void    ToShaderResourceState()     override;
    
    /// <summary>
    /// デバッグオーバーレイを描画します。
    /// </summary>
    void    DebugOverlay()              override;

    // Setters
    /// <summary>
    /// 入力テクスチャの GPU ハンドルを設定します。
    /// </summary>
    /// <param name="gpuHandle">SRV ハンドル。</param>
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) override;

    // =============================================
    // [Getter Begin]
    D3D12_GPU_DESCRIPTOR_HANDLE             GetOutputTextureHandle()        const override;
    const std::string&                      GetName()                       const override;
    /// <summary>適用後のリソースを取得します。</summary>
    DX12Resource*                           GetOutputResource()             const override;
    /// <summary>オプション（読み取り専用）を取得します。</summary>
    const GaussianBloomOption&              GetOption()                     const;
    /// <summary>内部の分離ガウシアンフィルタ（読み取り専用）を取得します。</summary>
    const SeparatedGaussianFilter*          GetSeparatedGaussianFilter()    const;
    /// <summary>内部の輝度抽出フィルタ（読み取り専用）を取得します。</summary>
    const LuminanceOutput*                  GetLuminanceOutputFilter()      const;
    /// <summary>オプションへの参照を取得します。</summary>
    GaussianBloomOption&                    GetOption();
    /// <summary>内部の分離ガウシアンフィルタへの参照を取得します。</summary>
    SeparatedGaussianFilter*                GetSeparatedGaussianFilter();
    /// <summary>内部の輝度抽出フィルタへの参照を取得します。</summary>
    LuminanceOutput*                        GetLuminanceOutputFilter();
    // [Getter End]
    // =============================================

    // =============================================
    // [Option Begin]
    void    SetKernelSize(int size);
    void    SetSigma(float sigma);
    void    SetThreshold(float threshold);
    void    SetBloomIntensity(float intensity);
    // [Option End]
    // =============================================


private:
    DirectX12*                                          pDx12_                      = nullptr;
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
    // [Final Output End]
    // =============================================

    // =============================================
    // [ConstantBuffers Begin]
    Microsoft::WRL::ComPtr<ID3D12Resource>              cbOptionResorce_            = nullptr;
    GaussianBloomOption*                                cbOptionData_               = nullptr;
    // [ConstantBuffers End]
    // =============================================

    const PSOID                                         kPSOId_                     = "GaussianBloom";
    const RootSignatureID                               kRootSignatureId_           = "GaussianBloom";
    const std::wstring                                  kVertexShaderPath           = L"EngineResources/Shaders/GaussianBloom.VS.hlsl";
    const std::wstring                                  kPixelShaderPath            = L"EngineResources/Shaders/GaussianBloom.PS.hlsl";

    ID3D12PipelineState*                                pso_                        = nullptr;
    ID3D12RootSignature*                                rootSignature_              = nullptr;

    // Internal functions
    void    RegisterRootSignature();
    void    CreatePipelineStateObject();
    void    PreDrawSetting(D3D12_GPU_DESCRIPTOR_HANDLE inputGpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE outputCpuHandle);
    void    InitializeLuminanceOutputFilter();
    void    InitializeSeparatedGaussianFilter();
    void    CreateResourceCBuffer();
    D3D12_GPU_DESCRIPTOR_HANDLE     ApplyFilter(D3D12_GPU_DESCRIPTOR_HANDLE inputGpuHandle, IPostEffect* pEffect);
};