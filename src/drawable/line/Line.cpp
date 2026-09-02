#include "Line.h"

#include <Core/DirectX12/Helper/DX12Helper.h>

Line::~Line()
{
}

void Line::Initialize()
{
    /// 必要なインスタンスを取得
    pLineSystem_ = LineSystem::GetInstance();
    pDx12_ = pLineSystem_->GetDirectX12();
    device_ = pDx12_->GetDevice();

    ppGameEye_ = pLineSystem_->GetGlobalEye();

    if (vertices_.size() == 0)
    {
        vertices_.resize(2);
    }

    transform_ = {
        .scale      = Vector3(1.0f, 1.0f, 1.0f),
        .rotate     = Vector3(0.0f, 0.0f, 0.0f),
        .translate  = Vector3(0.0f, 0.0f, 0.0f),
    };

    CreateVertexResource();
    CreateWVPMatrixResource();
    CreateColorResource();
}

void Line::Finalize()
{
    OutputDebugStringA("Line::Finalize\n");
}

void Line::Update()
{
    Matrix4x4 wMatrix = Matrix4x4::AffineMatrix(
        transform_.scale,
        transform_.rotate,
        transform_.translate
    );

    Matrix4x4 vpMatrix = (*ppGameEye_)->GetViewProjectionMatrix();

    pWVPMatrixData_[0] = wMatrix * vpMatrix;

    for (size_t i = 0; i < vertices_.size(); ++i)
    {
        pVertexData_[i] = vertices_[i];
    }

    pColorData_[0] = color_;
}

void Line::DrawCall(ID3D12GraphicsCommandList* cl)
{
    LineSystem::CommandListData clData;
    clData.vertexCount = static_cast<uint32_t>(vertices_.size());
    clData.rtvHandle = DrawableBase::GetRTVHandleCPU();
    clData.cbuffers[0] = colorResource_.Get();
    clData.cbuffers[1] = wvpMatrixResource_.Get();
    clData.vbView = vertexBufferView_;
    pLineSystem_->DrawSingle(cl, clData);
}

void Line::Resize(size_t size)
{
    vertices_.resize(size);
    vertexResource_.Reset();
    CreateVertexResource();
}

void Line::ResizeLine(size_t numLines)
{
    this->Resize(numLines * 2);
}

void Line::CreateVertexResource()
{
    /// 頂点バッファリソースを作成
    vertexResource_ = DX12Helper::CreateBufferResource(device_, sizeof(Vector3) * vertices_.size());
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&pVertexData_));
    /// 頂点データを初期化
    for (size_t i = 0; i < vertices_.size(); ++i)
    {
        pVertexData_[i] = vertices_[i];
    }

    /// 頂点バッファービューを初期化
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<uint32_t>(sizeof(Vector3) * vertices_.size());
    vertexBufferView_.StrideInBytes = sizeof(Vector3);
}

void Line::CreateWVPMatrixResource()
{
    /// 座標変換行列リソースを作成
    wvpMatrixResource_ = DX12Helper::CreateBufferResource(device_, sizeof(Matrix4x4));
    wvpMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&pWVPMatrixData_));
    /// 座標変換行列データを初期化
    pWVPMatrixData_[0] = Matrix4x4::Identity();
}

void Line::CreateColorResource()
{
    /// 色リソースを作成
    colorResource_ = DX12Helper::CreateBufferResource(device_, sizeof(Vector4));
    colorResource_->Map(0, nullptr, reinterpret_cast<void**>(&pColorData_));
    /// 色データを初期化
    pColorData_[0] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}
