#pragma once

#include <Effects/PostEffects/IPostEffect.h>
#include <Core/DirectX12/PipelineStateObject/PSOCache.h>
#include "Core/DirectX12/RootSignature/RootSignatureCache.h"

/// ポストエフェクトクラス 共通部分実装クラス
/// 一部の関数は実装されていないため、継承先でオーバーライドする必要があります。
class PostEffectBase : public IPostEffect
{
public:
    /// PSO ID や ルートシグネチャIDなどの必須情報は、Initialize前に継承先で設定してください。
    void Initialize(const PostEffectInitParams& desc) override;
    void Finalize() override;

    inline  bool                            Enabled() const override { return isEnabled_; }
    inline  void                            SetInputTextureHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle) override { inputGpuHandle_ = handle; }
    inline  D3D12_GPU_DESCRIPTOR_HANDLE     GetOutputTextureHandle() const override { return pRenderTexture_->GetSRVHandleGPU(); }
    inline  const std::string&              GetName() const override { return name_; }
    inline  DX12Resource*                   GetOutputResource() const override { return pRenderTexture_.get(); }
    virtual void                            Setting() override;
            void                            Apply() override;
            void                            ToShaderResourceState() override;
    inline  void                            Enable(bool flag) override { isEnabled_ = flag; }

protected:
    /// PSO IDを設定します。(必須)
    inline  void                            SetPSOId(const PSOID& psoId) { psoId_ = psoId; }
    /// ルートシグネチャIDを設定します。(必須)
    inline  void                            SetRootSignatureId(const RootSignatureID& rootSignatureId) { rootSignatureId_ = rootSignatureId; }
    /// エフェクト名を設定します。
    inline  void                            SetName(const std::string& name) { name_ = name; }
    /// 頂点シェーダーのパスを設定します
    inline  void                            SetVertexShaderPath(const std::wstring& path) { vertexShaderPath_ = path; }
    /// ピクセルシェーダーのパスを設定します (必須)
    inline  void                            SetPixelShaderPath(const std::wstring& path) { pixelShaderPath_ = path; }
    /// <summary>定数バッファを生成し、初期化します。</summary>
    virtual void                            CreateCBuffer(ID3D12Device* pDevice) {};
    /// <summary>定数バッファをシェーダーにセットします。(Indexは2からスタートしてください)</summary>
    virtual void                            SetCBuffer(ID3D12GraphicsCommandList* pCommandList) {};
    /// <summary>追加のルートパラメータを登録します。(Indexは2からスタートしてください)</summary>
    virtual void                            RegisterAdditionalRootParameter(RootParameters& rootParams) {};

private:
    void    RegisterRootSignature();
    void    CreatePipelineStateObject();
    void    CheckInvariants();

    std::string                             name_               = "unnamed";
    PSOID                                   psoId_;
    RootSignatureID                         rootSignatureId_;
    std::wstring                            vertexShaderPath_   = L"EngineResources/Shaders/PostEffectCommon.VS.hlsl";
    std::wstring                            pixelShaderPath_;

    DirectX12*                              pDx12_              = nullptr;
    ID3D12Device*                           pDevice_            = nullptr;
    ID3D12GraphicsCommandList*              pCommandList_       = nullptr;

    bool                                    isEnabled_          = false;
    std::unique_ptr<DX12Resource>           pRenderTexture_     = {};
    ID3D12PipelineState*                    pPSO_               = nullptr;
    ID3D12RootSignature*                    pRootSignature_     = nullptr;
    D3D12_GPU_DESCRIPTOR_HANDLE             inputGpuHandle_     = {};
};