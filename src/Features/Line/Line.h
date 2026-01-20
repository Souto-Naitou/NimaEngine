#pragma once

#include "LineSystem.h"
#include <d3d12.h>
#include <Matrix4x4.h>
#include <Vector4.h>
#include <wrl/client.h>
#include <Core/DirectX12/DirectX12.h>
#include <vector>
#include <Features/GameEye/GameEye.h>
#include <drawable/base/DrawableBase.h>
#include <Math/Transform.h>

/// <summary>
/// ライン描画クラス
/// </summary>
class Line : public DrawableBase
{
public:
    Line(size_t lineCount) { vertices_.resize(lineCount * 2); }
    ~Line();
    /// <summary>
    /// ライン描画に必要なGPUリソース等を初期化します。
    /// </summary>
    void Initialize();
    
    /// <summary>
    /// リソースの解放を行います。
    /// </summary>
    void Finalize();
    
    /// <summary>
    /// 行列や頂点など内部状態の更新を行います。
    /// </summary>
    void Update();
    
    /// <summary>
    /// ラインを描画します。
    /// </summary>
    void DrawCall(ID3D12GraphicsCommandList* cl) override;

public:
    Vector3& operator [](size_t idx) { return vertices_[idx]; }


public:
    /// <summary>
    /// 頂点配列サイズを変更します（2頂点で1ライン）。
    /// </summary>
    /// <param name="size">頂点数。</param>
    void Resize(size_t size);
    /// <summary>
    /// ライン本数を変更します。
    /// </summary>
    /// <param name="numLines">ライン数。</param>
    void ResizeLine(size_t numLines);
    void SetColor(const Vector4& color) { color_ = color; }
    void SetGameEye(GameEye** eye) { pGameEye_ = eye; }
    void SetTransform(const EulerTransform& transform) { transform_ = transform; }


public: /// Getter
    Vector3& GetVertex(size_t idx) { return vertices_[idx]; }
    std::vector<Vector3>& GetVertices() { return vertices_; }
    Vector4& GetColorData() { return color_; }
    size_t GetLineCount() const { return vertices_.size() / 2; }
    const EulerTransform& GetTransform() const { return transform_; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    Vector3* pVertexData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> wvpMatrixResource_;
    Matrix4x4* pWVPMatrixData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> colorResource_;
    Vector4* pColorData_ = nullptr;

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};

    std::vector<Vector3> vertices_ = {};
    Vector4 color_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    EulerTransform transform_ = {};

private:
    /// <summary>
    /// 頂点バッファ用のリソースを生成します。
    /// </summary>
    void CreateVertexResource();
    
    /// <summary>
    /// WVP 行列用のリソースを生成します。
    /// </summary>
    void CreateWVPMatrixResource();
    
    /// <summary>
    /// 色定数バッファ用のリソースを生成します。
    /// </summary>
    void CreateColorResource();

private:
    LineSystem* pLineSystem_ = nullptr;
    DirectX12* pDx12_ = nullptr;
    ID3D12Device* device_ = nullptr;
    GameEye** pGameEye_ = nullptr;
};