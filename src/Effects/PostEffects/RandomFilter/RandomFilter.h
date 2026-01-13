#pragma once

#include <Effects/PostEffects/IPostEffect.h>
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

    // オプションの設定
    /// <summary>不透明度を設定します。</summary>
    void    SetOpacity(float opacity);
    /// <summary>シード値を設定します。</summary>
    void    SetSeed(float seed);

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

    // Setters
    /// <summary>入力テクスチャのハンドルを設定します。</summary>
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) override;

    // Getters
    D3D12_GPU_DESCRIPTOR_HANDLE     GetOutputTextureHandle() const override;
    const std::string&              GetName() const override;
    /// <summary>適用後のリソースを取得します。</summary>
    DX12Resource*     GetOutputResource() const override;
    /// <summary>オプションへの参照を取得します。</summary>
    RandomFilterOption&             GetOption();
    /// <summary>オプション（読み取り専用）を取得します。</summary>
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