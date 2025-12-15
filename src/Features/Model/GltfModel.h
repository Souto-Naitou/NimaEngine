#pragma once

#include <Core/DirectX12/ResourceStateTracker/ResourceStateTracker.h>
#include <Features/Model/IModel.h>
#include <Features/Model/ModelData.h>
#include <Features/Model/Animation.h>
#include <Features/Model/Skeleton.h>
#include <Features/Model/SkinCluster.h>
#include <wrl/client.h>
#include <Features/TimeMeasurer/TimeMeasurer.h>
#include "SkinCluster.h"

/// <summary>
/// glTFモデル
/// </summary>
class GltfModel : public IModel 
{
public:
    // Common functions
    ~GltfModel()                                override;
    void Initialize()                           override;
    void Finalize()                             override;
    void Update()                               override;
    void Draw(ID3D12GraphicsCommandList* cl)   override;
    void CreateGPUResource()                    override;
    void Clone(IModel* src)                    override;
    std::unique_ptr<IModel> Cloned()            override;
    void DispatchSkinning();

    // Getter
    D3D12_VERTEX_BUFFER_VIEW    GetVertexBufferView()   const   override;
    size_t                      GetVertexCount()        const   override;
    D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandle()   const   override;
    bool                        IsEndLoading()          const   override;
    ModelData*                  GetModelData()                  override;
    Animation*                  GetAnimationData();
    Skeleton*                   GetSkeleton();
    SkinCluster*                GetSkinCluster();

    // Setter
    void ChangeTexture(D3D12_GPU_DESCRIPTOR_HANDLE texSrvHnd) override;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource>  vertexResource_         = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>  indexResource_          = nullptr;  //< インデックスバッファリソース
    DX12Resource                            resourceSkinned_        = {};
    ModelData                               modelData_              = {};
    Skeleton                                skeleton_;                          //< スケルトンデータ
    Animation                               animationData_          = {};
    SkinCluster                             skinCluster_;                        //< スキンクラスター
    D3D12_VERTEX_BUFFER_VIEW                vertexBufferView_       = {};
    D3D12_INDEX_BUFFER_VIEW                 indexBufferView_        = {};
    VertexData*                             vertexData_             = nullptr;  //< 頂点データのポインタ
    uint32_t*                               indexData_              = nullptr;  //< インデックスデータのポインタ
    bool                                    isReadyDraw_            = false;
    GltfModel*                              pCloneSrc_              = nullptr;  //< クローン元のインスタンス
    bool                                    isOverwroteTexture_     = false;    //< テクスチャを上書きしたかどうか
    TimeMeasurer                            timer_                  = {};       //< タイマー
    D3D12_GPU_DESCRIPTOR_HANDLE             textureSrvHandleGPU_    = {};

    // SRV
    D3D12_GPU_DESCRIPTOR_HANDLE             srvHandleGpuInputVertex_    = {};
    D3D12_GPU_DESCRIPTOR_HANDLE             srvHandleGpuSkinned_        = {};
    uint32_t                                srvIndexInputVertex_        = 0u;       //< SRVのインデックス（頂点用）
    uint32_t                                srvIndexSkinned_            = 0u;       //< UAVのインデックス

    // Pointers
    SRVManager*                             srvManager_                 = nullptr;  //< SRVマネージャー
    
    #ifdef _DEBUG
    bool    is_called_finalize_    = true;     //< Finalizeが呼ばれたかどうか
    #endif // _DEBUG

    // Internal functions
    void    CreateVertexResource();
    void    CreateIndexResource();
    void    CreateSkinnedResource();
    void    _LoadModelTexture();
    void    CopyFrom(GltfModel* _pCopySrc);
    void    _UpdateLocalMatrixByAnimation();
    void    _UpdateSkeleton();
    void    _UpdateSkinCluster();
    void    _ApplyAnimationToSkeleton();
    void    CreateUAV();
};