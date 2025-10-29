#pragma once

#include <Vector4.h>
#include <Vector2.h>
#include <Core/DirectX12/DirectX12.h>
#include <Math/Transform.h>
#include <Common/structs.h>
#include <Features/Model/ModelData.h>
#include <string>
#include <DirectXTex/DirectXTex.h>
#include <DebugTools/DebugEntry/DebugEntry.h>
#include <memory>

class SpriteSystem;


class Sprite
{
public:
    struct Material
    {
        Vector4     color;
        Matrix4x4   uvTransform;
    };

    Sprite();
    ~Sprite();
    
    /// <summary>
    /// スプライトを初期化します。
    /// </summary>
    /// <param name="_filepath">使用するテクスチャのパス。</param>
    void                Initialize(std::string _filepath);
    
    /// <summary>
    /// スプライトの状態更新を行います。
    /// </summary>
    void                Update();
    
    /// <summary>
    /// スプライトを描画します。
    /// </summary>
    void                Draw();
    
    /// <summary>
    /// リソースの解放を行います。
    /// </summary>
    void                Finalize();
    
    /// <summary>
    /// デバッグUIを描画します。
    /// </summary>
    void                ImGui();


public: /// Getter
    const bool          GetEnableUpdate()   const                   { return isUpdateEnabled_; }
    const bool          GetEnableDraw()     const                   { return isDrawEnabled_; }
    const float         GetRotation()       const                   { return rotate_; }
    const Vector2&      GetPosition()       const                   { return translate_; }
    const Vector4&      GetColor()          const                   { return materialData_->color; }
    const Vector2&      GetSize()           const                   { return size_; }
    const Vector2&      GetAnchorPoint()    const                   { return anchorPoint_; }
    const bool          GetFlipX()          const                   { return isFlipX_; }
    const bool          GetFlipY()          const                   { return isFlipY_; }
    const Vector2&      GetTextureLeftTop() const                   { return textureLeftTop_; }
    const Vector2&      GetTextureSize()    const                   { return textureSize_; }

public: /// Setter
    void                SetName(const std::string& name)            { pDebugEntry_->SetName(name); }
    void                SetEnableUpdate(const bool isUpdate)        { isUpdateEnabled_ = isUpdate; }
    void                SetEnableDraw(const bool isDraw)            { isDrawEnabled_ = isDraw; }
    void                SetRotation(const float rotation)           { rotate_ = rotation; }
    void                SetPosition(const Vector2& position)        { translate_ = position; }
    void                SetColor(const Vector4& color)              { materialData_->color = color; }
    void                SetSize(const Vector2& size)                { size_ = size; }
    /// <summary>
    /// 元サイズに対する倍率でサイズを設定します。
    /// </summary>
    /// <param name="factor">倍率。</param>
    void                SetSizeWithFactor(float factor);
    void                SetAnchorPoint(const Vector2& anchor)      { anchorPoint_ = anchor; }
    void                SetFlipX(const bool isFlipX)               { isFlipX_ = isFlipX; }
    void                SetFlipY(const bool isFlipY)               { isFlipY_ = isFlipY; }
    void                SetTextureLeftTop(const Vector2& lt)       { textureLeftTop_ = lt; }
    void                SetTextureSize(const Vector2& size)        { textureSize_ = size; }
    void                SetRTVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle) { rtvHandleCPU_ = rtvHandle; }


private: /// 他クラスが所持するインスタンスへのポインタ
    SpriteSystem*                               pSpriteSystem_                  = nullptr;
    DirectX12*                                  pDx12_                          = nullptr;
    ID3D12Device*                               device_                         = nullptr;


private: /// メンバ変数
    std::unique_ptr<DebugEntry<Sprite>>         pDebugEntry_                    = nullptr;              // デバッグエントリ
    bool                                        isUpdateEnabled_                = true;                 // 有効かどうか。falseにするとUpdateとDrawをスキップする
    bool                                        isDrawEnabled_                  = true;                 // 描画するかどうか。falseにするとDrawをスキップする

    EulerTransform                              transform_                      = {};                   // 位置、回転、拡大縮小
    Vector2                                     size_                           = {};                   // サイズ
    float                                       rotate_                         = {};                   // 回転角
    Vector2                                     translate_                      = {};                   // 位置
    Vector2                                     anchorPoint_                    = {};                   // アンカーポイント
    bool                                        isFlipX_                        = false;                // X軸反転
    bool                                        isFlipY_                        = false;                // Y軸反転
    float                                       aspectRatio_                    = 0.0f;                 // アスペクト比

    Vector2                                     textureLeftTop_                 = {};                   // テクスチャの左上uv
    Vector2                                     textureSize_                    = {};                   // テクスチャのuv範囲
    EulerTransform                              uvTransform_                    = {};                   // UV変換
    Matrix4x4                                   uvTransformMatrix_              = {};                   // UV変換行列

    std::string                                 texturePath_                    = {};                   // テクスチャファイルパス

    /// ディスクリプタハンドル
    D3D12_GPU_DESCRIPTOR_HANDLE                 textureSrvHandleGPU_            = {};                   // テクスチャハンドルGPU
    D3D12_CPU_DESCRIPTOR_HANDLE                 rtvHandleCPU_                   = {};                   // RTVハンドルCPU

    /// バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertexResource_                 = nullptr;              // 頂点リソース
    Microsoft::WRL::ComPtr<ID3D12Resource>      indexResource_                  = nullptr;              // インデックスリソース

    /// バッファリソースのデータ
    VertexData*                                 vertexData_                     = nullptr;              // 頂点データ
    uint32_t*                                   indexData_                      = nullptr;              // インデックスデータ

    /// バッファリソースの使い方を示すビュー
    D3D12_VERTEX_BUFFER_VIEW                    vertexBufferView_               = {};                   // 頂点バッファビュー
    D3D12_INDEX_BUFFER_VIEW                     indexBufferView_                = {};                   // インデックスバッファービュー

    /// マテリアル
    Microsoft::WRL::ComPtr<ID3D12Resource>      materialResource_               = nullptr;              // マテリアルリソース
    Material*                                   materialData_                   = nullptr;              // マテリアルデータ

    /// 変換行列
    Microsoft::WRL::ComPtr<ID3D12Resource>      transformationMatrixResource_   = nullptr;              // 変換行列リソース
    TransformationMatrix*                       transformationMatrixData_       = nullptr;              // 変換行列データ

    /// メタデータ
    DirectX::TexMetadata                        metadata_                       = {};                   // テクスチャのメタデータ

    /// ImGui用
    #ifdef _DEBUG
    Vector2                                     thumbnailSize_                  = { 100.0f, 100.0f };   // サムネイルサイズ
    bool                                        isUpdateCalled_                 = false;                // 更新処理が呼ばれたかどうか
    #endif

private: /// メンバ関数
    /// <summary>
    /// 頂点バッファ用リソースを作成します。
    /// </summary>
    void CreateVertexResource();
    
    /// <summary>
    /// インデックスバッファ用リソースを作成します。
    /// </summary>
    void CreateIndexResource();
    
    /// <summary>
    /// 頂点バッファビューを作成します。
    /// </summary>
    void CreateVertexBufferView();
    
    /// <summary>
    /// インデックスバッファビューを作成します。
    /// </summary>
    void CreateIndexBufferView();

    /// <summary>
    /// 頂点バッファをCPU側にマップします。
    /// </summary>
    void MapVertexData();
    
    /// <summary>
    /// インデックスバッファをCPU側にマップします。
    /// </summary>
    void MapIndexData();

    /// <summary>
    /// マテリアル用定数バッファを作成します。
    /// </summary>
    void CreateMaterialResource();
    
    /// <summary>
    /// 変換行列用定数バッファを作成します。
    /// </summary>
    void CreateTransformationMatrixResource();
    
    /// <summary>
    /// テクスチャのアスペクトなどから適切なスプライトサイズに調整します。
    /// </summary>
    void AdjustSpriteSize();
};