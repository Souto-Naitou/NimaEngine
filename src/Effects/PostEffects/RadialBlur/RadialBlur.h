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

/// ラジアルブラー
class RadialBlur : public IPostEffect
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

    /// <summary>
    /// 有効・無効を設定します。
    /// </summary>
    /// <param name="_flag">true で有効。</param>
    void    Enable(bool _flag) override;
    /// <summary>
    /// 有効かどうかを返します。
    /// </summary>
    bool    Enabled() const override;

    // Setter (Additional)
    /// <summary>ブラーの中心位置を設定します（0.0～1.0）。</summary>
    void    SetCenter(float _center) { pOption_->center = _center; }
    /// <summary>サンプル数を設定します。</summary>
    void    SetSamples(int _sample) { pOption_->samples = _sample; }
    /// <summary>ブラー幅を設定します。</summary>
    void    SetBlurWidth(float _width) { pOption_->blurWidth = _width; }

    /// <summary>
    /// エフェクトを適用します。
    /// </summary>
    void    Apply() override;
    /// <summary>
    /// 描画設定を行います。
    /// </summary>
    void    Setting() override;
    /// <summary>
    /// リサイズ前の処理を行います。
    /// </summary>
    void    OnResizeBefore() override;
    /// <summary>
    /// リサイズ後の処理を行います。
    /// </summary>
    void    OnResizeAfter() override;
    /// <summary>
    /// レンダーターゲットをシェーダーリソース状態へ遷移させます。
    /// </summary>
    void    ToShaderResourceState() override;
    /// <summary>
    /// デバッグオーバーレイを描画します。
    /// </summary>
    void    DebugOverlay() override;

    // Setters
    /// <summary>
    /// 入力テクスチャのハンドルを設定します。
    /// </summary>
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle) override;

    // =============================================
    // [Getters Begin]
    D3D12_GPU_DESCRIPTOR_HANDLE     GetOutputTextureHandle() const override;
    const std::string&              GetName() const override;
    /// <summary>オプションへの参照を取得します。</summary>
    RadialBlurOption&               GetOption();
    /// <summary>オプション（読み取り専用）を取得します。</summary>
    const RadialBlurOption&         GetOption() const;
    // [Getters End]
    // =============================================


private:
    DirectX12*                                          pDx12_                  = nullptr;
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;

    bool                                                isEnabled_              = false;
    const std::string                                   name_                   = "RadialBlur";
    DX12Resource                                        renderTexture_          = {};
    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_       = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_        = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>         pso_                    = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_          = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
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