#pragma once

#pragma warning(disable: 4324) // 構造体のメンバがアライメントのためにパディングされる

#include "LineSystem.h"
#include <d3d12.h>
#include <Matrix4x4.h>
#include <Vector4.h>
#include <wrl/client.h>
#include <Core/DirectX12/DirectX12.h>
#include <vector>
#include <Features/GameEye/GameEye.h>

/// <summary>
/// ライン描画クラス
/// </summary>
class Line
{
public:
    Line(size_t _lineCount) { vertices_.resize(_lineCount * 2); }
    ~Line();

    void Initialize();
    void Finalize();
    void Update();
    void Draw();

public:
    Vector3& operator [](size_t _idx) { return vertices_[_idx]; }


public:
    void Resize(size_t _size);
    void ResizeLine(size_t _numLines);
    void SetColor(const Vector4& _color) { color_ = _color; }
    void SetGameEye(GameEye** _eye) { pGameEye_ = _eye; }


public: /// Getter
    Vector3& GetVertex(size_t _idx) { return vertices_[_idx]; }
    std::vector<Vector3>& GetVertices() { return vertices_; }
    Vector4& GetColorData() { return color_; }
    size_t GetLineCount() const { return vertices_.size() / 2; }


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

private:
    void CreateVertexResource();
    void CreateWVPMatrixResource();
    void CreateColorResource();

private:
    LineSystem* pLineSystem_ = nullptr;
    DirectX12* pDx12_ = nullptr;
    ID3D12Device* device_ = nullptr;
    GameEye** pGameEye_ = nullptr;
};