#pragma once

#include <Effects/PostEffects/IPostEffect.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <Core/DirectX12/DirectX12.h>
#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>
#include <string>
#include <Core/DirectX12/DX12Resource/DX12Resource.h>
#include <Vector4.h>
#include <Core/DirectX12/RootSignature/RootSignatureCache.h>
#include <Core/DirectX12/PipelineStateObject/PSOCache.h>

struct alignas(16) DissolveOption
{
    float threshold;
    float edgeThresholdOffset;
    float padding[2];
    Vector4 colorDissolve;
    Vector4 colorEdge;
};

/// ディゾルブ
class Dissolve : public IPostEffect
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

    /// <summary>
    /// ディゾルブ用のマスクテクスチャを設定します。
    /// </summary>
    /// <param name="texResource">テクスチャリソース。</param>
    void    SetTextureResource(const DX12Resource& texResource);

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
    const std::string&          GetName() const override;
    /// <summary>適用後のリソースを取得します。</summary>
    DX12Resource*       GetOutputResource() const override;
    /// <summary>オプションへの参照を取得します。</summary>
    DissolveOption&              GetOption();
    /// <summary>オプション（読み取り専用）を取得します。</summary>
    const DissolveOption&           GetOption() const;

private:
    const PSOID                                         kPSOId_                 = "Dissolve";
    const RootSignatureID                               kRootSignatureId_       = "Dissolve";
    const std::string                                   name_                   = "Dissolve";
    const std::wstring                                  kVertexShaderPath       = L"EngineResources/Shaders/Dissolve.VS.hlsl";
    const std::wstring                                  kPixelShaderPath        = L"EngineResources/Shaders/Dissolve.PS.hlsl";

    DirectX12*                                          pDx12_                  = nullptr;
    ID3D12Device*                                       device_                 = nullptr;
    ID3D12GraphicsCommandList*                          commandList_            = nullptr;

    bool                                                isEnabled_              = false;
    DX12Resource                                        renderTexture_          = {};
    ID3D12PipelineState*                                pso_                    = nullptr;
    ID3D12RootSignature*                                rootSignature_          = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE                         inputGpuHandle_         = {};
    DX12Resource                                        maskTexture_            = {};

    // Constant buffers
    Microsoft::WRL::ComPtr<ID3D12Resource>              optionResource_         = nullptr;
    DissolveOption*                                     pOption_                = nullptr;

    // Internal functions
    void    RegisterRootSignature();
    void    CreatePipelineStateObject();
    void    CreateResourceCBuffer();
    void    CheckValidation() const;
};