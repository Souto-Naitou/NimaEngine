#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <list>

#include <BaseClasses/ObjectSystemBaseMT.h>
#include <Common/SingletonPattern.h>
#include <drawable/object3d/Object3dSystem.h>

/// <summary>
/// スプライト描画共通
/// </summary>
class SpriteSystem : 
    public SingletonPattern<SpriteSystem>,
    public ObjectSystemBaseMT
{
    friend class SingletonPattern<SpriteSystem>;

public:
    struct CommandListData
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandleCPU;
        ID3D12Resource* materialResource;
        ID3D12Resource* transformationMatrixResource;
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;
        D3D12_VERTEX_BUFFER_VIEW* pVBV;
        D3D12_INDEX_BUFFER_VIEW* pIBV;
    };

    // Common functions
    /// <summary>
    /// スプライト描画システムを初期化します。
    /// </summary>
    void Initialize() override;
    /// <summary>
    /// バックバッファへの描画を実施します。
    /// </summary>
    void PresentDraw();
    /// <summary>
    /// 累積した描画コマンドを発行します。
    /// </summary>
    void DrawCall();
    /// <summary>
    /// GPU 完了を待機して同期します。
    /// </summary>
    void Sync();

    // Setter
    /// <summary>
    /// 描画に必要なコマンドリストデータを追加します。
    /// </summary>
    /// <param name="data">マテリアル、行列、VBV/IBV、SRV など。</param>
    void AddCommandListData(const CommandListData& data);

    void DrawSingle(ID3D12GraphicsCommandList* commandList, SpriteSystem::CommandListData& data);

private:
    void CreateRootSignature();
    void CreatePipelineState();

    // DirectX objects and paths
    static constexpr wchar_t                    kVertexShaderPath[]     = L"EngineResources/Shaders/Sprite.VS.hlsl";
    static constexpr wchar_t                    kPixelShaderPath[]      = L"EngineResources/Shaders/Sprite.PS.hlsl";
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_          = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState_  = nullptr;
    std::list<CommandListData>                  commandListDatas_       = {};
};