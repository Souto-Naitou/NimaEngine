#pragma once

#include <Core/DirectX12/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/PipelineStateObject/PipelineStateObject.h>

struct BoxFilterOption
{
    unsigned int kernelSize = 3;
};

/// <ボックスフィルタ>
/// - ApplyメソッドとSettingメソッドはPostEffectクラスで実行する
class BoxFilter : public IPostEffect
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
    void    Enable(bool _flag) override;
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

    // Setters
    /// <summary>入力テクスチャのハンドルを設定します。</summary>
    void    SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle) override;

    // Getters
    D3D12_GPU_DESCRIPTOR_HANDLE     GetOutputTextureHandle() const override;
    const std::string&              GetName() const override;
    /// <summary>オプションへの参照を取得します。</summary>
    BoxFilterOption&                GetOption();
    /// <summary>オプション（読み取り専用）を取得します。</summary>
    const BoxFilterOption&          GetOption() const;

private:
    DirectX12*                                          pDx12_                  = nullptr;
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;

    bool                                                isEnabled_              = false;
    const std::string                                   name_                   = "BoxFilter";
    DX12Resource                                        renderTexture_          = {};
    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_       = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_        = nullptr;
    PipelineStateObject                                 pso_                    = {};
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_          = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE                         rtvHandleCpu_           = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         rtvHandleGpu_           = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
    uint32_t                                            rtvHeapIndex_           = 0;
    uint32_t                                            srvHeapIndex_           = 0;
    const std::wstring                                  kVertexShaderPath       = L"EngineResources/Shaders/BoxFilter.VS.hlsl";
    const std::wstring                                  kPixelShaderPath        = L"EngineResources/Shaders/BoxFilter.PS.hlsl";

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D12Resource>              optionResource_         = nullptr;
    BoxFilterOption*                                    pOption_                = nullptr;

    // Internal functions
    void    CreateRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
};