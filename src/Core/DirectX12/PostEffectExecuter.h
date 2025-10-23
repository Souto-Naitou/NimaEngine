#pragma once

#include "DirectX12.h"
#include "SRVManager.h"
#include "./IPostEffect.h"
#include "DX12Resource/DX12Resource.h"
#include <Effects/PostEffects/.Factory/PostEffectFactory.h>

#include <wrl/client.h>
#include <vector>
#include <functional>
#include <DebugTools/DebugEntry/DebugEntry.h>

/// <ポストエフェクトを実行するクラス>
/// - 複数のポストエフェクトを順に適用するためにレンダーテクスチャのチェインを生成する
/// - Bloom -> MotionBlur の順で実行する場合 MotionBlurにBloomのレンダーテクスチャを渡す
class PostEffectExecuter : public EngineFeature
{
public:
    /// ctor , dtor
    PostEffectExecuter() = default;
    ~PostEffectExecuter() = default;
    
    /// <summary>
    /// 実行に必要なリソース・パイプラインを初期化します。
    /// </summary>
    /// <param name="isRegisterDebugWindow">デバッグUIを登録するか。</param>
    void Initialize(bool isRegisterDebugWindow = true);
    /// <summary>
    /// リソースを解放します。
    /// </summary>
    void Finalize();
    /// <summary>
    /// 登録されたポストエフェクトを順に適用します。
    /// </summary>
    void ApplyPostEffects();
    /// <summary>
    /// フレーム開始時の初期化処理を行います。
    /// </summary>
    void NewFrame();
    /// <summary>
    /// ポストエフェクト適用後の後処理を行います。
    /// </summary>
    void PostDraw();
    /// <summary>
    /// 最終出力の描画を行います。
    /// </summary>
    void Draw();
    /// <summary>
    /// クライアントサイズ変更時に呼び出し、リソース再作成を要求します。
    /// </summary>
    void OnResize();
    /// <summary>
    /// バッファ再作成後に呼び出し、ハンドル等を更新します。
    /// </summary>
    void OnResizedBuffers();
    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void ImGui();


public:
    PostEffectExecuter& RegisterPostEffect(IPostEffect* postEffect)
    {
        postEffects_.emplace_back(postEffect);
        return *this;
    }

    PostEffectExecuter& UnregisterPostEffect(IPostEffect* postEffect)
    {
        auto it = std::remove(postEffects_.begin(), postEffects_.end(), postEffect);
        if (it != postEffects_.end())
        {
            postEffects_.erase(it, postEffects_.end());
        }
        return *this;
    }

    ID3D12Resource* GetRenderTexture() const
    {
        return renderTexture_.GetResource().Get();
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE* GetRTVHandle()
    {
        return &renderTexture_.GetRTVHandle();
    }

    ID3D12GraphicsCommandList* GetCommandList()
    {
        return commandListForDraw_.Get();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetRTVHandleGpu() const
    {
        return rtvHandleGpu_;
    }


private:
    static constexpr wchar_t kVertexShaderPath[] = L"EngineResources/Shaders/Fullscreen.VS.hlsl";
    static constexpr wchar_t kPixelShaderPath[] = L"EngineResources/Shaders/Fullscreen.PS.hlsl";
    DX12Resource                                        renderTexture_          = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         rtvHandleGpu_           = {};
    D3D12_GPU_DESCRIPTOR_HANDLE                         outputHandleGpu_        = {};
    D3D12_INPUT_LAYOUT_DESC                             inputLayoutDesc_        = {};
    D3D12_GRAPHICS_PIPELINE_STATE_DESC                  pipelineStateDesc_      = {};
    D3D12_RASTERIZER_DESC                               rasterizerDesc_         = {};
    Microsoft::WRL::ComPtr<ID3D12RootSignature>         rootSignature_          = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    vertexShaderBlob_       = nullptr;
    Microsoft::WRL::ComPtr<IDxcBlob>                    pixelShaderBlob_        = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>         pso_                    = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>   commandListForDraw_     = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>      commandAllocator_       = nullptr;
    std::unique_ptr<DebugEntry<PostEffectExecuter>>     pDebugEntry_            = nullptr;
    std::unique_ptr<PostEffectFactory>                  pEffectFactory_         = nullptr;

    std::vector<IPostEffect*>                           postEffects_           = {};


private:
    /// <summary>
    /// 必要なシステムインスタンスを取得します。
    /// </summary>
    void ObtainInstances();
    
    /// <summary>
    /// ルートシグネチャを作成します。
    /// </summary>
    void CreateRootSignature();
    
    /// <summary>
    /// パイプラインステートを作成します。
    /// </summary>
    void CreatePipelineState();
    
    /// <summary>
    /// 描画用コマンドリストを作成します。
    /// </summary>
    void CreateCommandList();
    
    /// <summary>
    /// 指定インデックスのエフェクトのみを単独実行するモードを切り替えます。
    /// </summary>
    /// <param name="_index">エフェクトのインデックス。</param>
    void EnableSolo(const size_t _index);
    
    /// <summary>
    /// 中央寄せテーブルにUIを配置するヘルパ。
    /// </summary>
    /// <param name="_fn">描画関数。</param>
    void ImGuiCenterTable(const std::function<void()>& _fn);


private:
    RTVHeapCounter*                         rtvHeapCounter_     = nullptr;
    ID3D12Device*                           pDevice_            = nullptr;
    SRVManager*                             pSRVManager_        = nullptr;
    ID3D12GraphicsCommandList*              commandListMain_    = nullptr;
    ID3D12DescriptorHeap*                   dsvHeap_            = {};
    ID3D12DescriptorHeap*                   rtvHeap_            = {};
    D3D12_CPU_DESCRIPTOR_HANDLE             rtvHandleSwapChain_ = {};
};