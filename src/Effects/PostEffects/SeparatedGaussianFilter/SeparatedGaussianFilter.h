#pragma once

#include <Effects/PostEffects/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>
#include <Core/DirectX12/RootSignature/RootSignatureCache.h>
#include <Core/DirectX12/PipelineStateObject/PSOCache.h>

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

/// 分離式ガウシアンフィルタ
class SeparatedGaussianFilter : public IPostEffect
{
public:
    /// <summary>
    /// エフェクトを初期化します。
    /// </summary>
    /// <param name="desc">DX12 初期化情報。</param>
    void    Initialize(const PostEffectInitParams& desc) override;
    
    /// <summary>
    /// リソースを解放します。
    /// </summary>
    void    Finalize() override;

    /// <summary>有効・無効を設定します。</summary>
    void    Enable(bool flag) override;
    /// <summary>有効かどうかを返します。</summary>
    bool    Enabled() const override;

    /// <summary>エフェクトを適用します。</summary>
    void    Apply() override;
    /// <summary>描画設定を行います。</summary>
    void    Setting() override;
    /// <summary>レンダーターゲットをシェーダーリソース状態へ遷移させます。</summary>
    void    ToShaderResourceState() override;
    /// <summary>デバッグオーバーレイを描画します。</summary>
    void    DebugOverlay() override;
    /// <summary>カーネルを生成します。</summary>
    void    CreateKernel();

    // Setters
    /// <summary>入力テクスチャのハンドルを設定します。</summary>
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) override;
    void    SetSigma(float sigma);

    // Getters
    D3D12_GPU_DESCRIPTOR_HANDLE     GetOutputTextureHandle() const override;
    const std::string&     GetName() const override;
    /// <summary>適用後のリソースを取得します。</summary>
    DX12Resource*    GetOutputResource() const override;
    /// <summary>オプションへの参照を取得します。</summary>
    SeparatedGaussianFilterOption&      GetOption();
    /// <summary>オプション（読み取り専用）を取得します。</summary>
    const SeparatedGaussianFilterOption&    GetOption() const;
    float GetSigma() const;

private:
    const PSOID                                         kPSOId_                     = "SeparatedGaussianFilter";
    const RootSignatureID                               kRootSignatureId_           = "SeparatedGaussianFilter";
    const std::string                                   name_                       = "SeparatedGaussianFilter";
    const std::wstring                                  kVertexShaderPath           = L"EngineResources/Shaders/SeparatedGaussianFilter.VS.hlsl";
    const std::wstring                                  kPixelShaderPath            = L"EngineResources/Shaders/SeparatedGaussianFilter.PS.hlsl";
    constexpr static const char*                        kNameHorizontal             = "RT_SeparatedGaussianFilter(Horizontal)";
    constexpr static const char*                        kNameVertical               = "RT_SeparatedGaussianFilter(Vertical)";

    DirectX12*                                          pDx12_                      = nullptr;
    ID3D12Device*                                       device_                     = nullptr;
    ID3D12GraphicsCommandList*                          commandList_                = nullptr;

    bool                                                isEnabled_                  = false;
    DX12Resource                                        horizontalGaussTexture_     = {};
    DX12Resource                                        renderTexture_              = {};
    ID3D12PipelineState*                                pso_                        = nullptr;
    ID3D12RootSignature*                                rootSignature_              = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_             = {};

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
    void    RegisterRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
    void    PreDrawSetting(D3D12_GPU_DESCRIPTOR_HANDLE inputGpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE outputCpuHandle, ID3D12Resource* execInfoResource);

};