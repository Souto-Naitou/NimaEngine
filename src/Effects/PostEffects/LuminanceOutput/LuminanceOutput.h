#pragma once

#include <Effects/PostEffects/IPostEffect.h>
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

/// 輝度抽出エフェクト
class LuminanceOutput : public IPostEffect
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
    /// <summary>リサイズ前の処理を行います。</summary>
    void    OnResizeBefore() override;
    /// <summary>リサイズ後の処理を行います。</summary>
    void    OnResizeAfter() override;
    /// <summary>レンダーターゲットをシェーダーリソース状態へ遷移させます。</summary>
    void    ToShaderResourceState() override;
    /// <summary>デバッグオーバーレイを描画します。</summary>
    void    DebugOverlay() override;

    /// [ Setters ]
    /// <summary>入力テクスチャのハンドルを設定します。</summary>
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) override;

    /// [ Getters ]
    D3D12_GPU_DESCRIPTOR_HANDLE     GetOutputTextureHandle() const override;
    const std::string&              GetName() const override;
    /// <summary>適用後のリソースを取得します。</summary>
    DX12Resource*                   GetOutputResource() const override;
    /// <summary>オプションへの参照を取得します。</summary>
    LuminanceOutputOption&          GetOption();
    /// <summary>オプション（読み取り専用）を取得します。</summary>
    const LuminanceOutputOption&    GetOption() const;



private:
    DirectX12*                                          pDx12_                  = nullptr;
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;
    bool                                                isEnabled_              = false;
    const std::string                                   name_                   = "LuminanceOutput";


    // 入力テクスチャハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};

    // 出力テクスチャハンドル
    DX12Resource                                        outputTexture_          = {};

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