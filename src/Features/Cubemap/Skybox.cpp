#include "Skybox.h"
#include <cstring>
#include <imgui.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <Core/DirectX12/Helper/DX12Helper.h>

void Skybox::Initialize(CubemapSystem* cms)
{
    pCubemapSystem_ = cms;
    pDx12_ = pCubemapSystem_->GetDirectX12();
    device_ = pDx12_->GetDevice();
    ppGlobalEye_ = pCubemapSystem_->GetGlobalEye();

    pDebugEntry_ = std::make_unique<DebugEntry<Skybox>>("Skybox", this, false);

    this->CreateVertexResource();
    this->CreateIndexResource();
    this->CreateTransformationMatrixResource();
    this->CreateMaterialResource();
}

void Skybox::Finalize() const
{
}

void Skybox::Update()
{
    // Update the transformation matrix if necessary.
    if (mappedTransformationMatrix_)
    {
        auto& tm = mappedTransformationMatrix_;
        tm->wvp = tm->world * (*ppGlobalEye_)->GetViewProjectionMatrix();
    }
}

void Skybox::Draw(ID3D12GraphicsCommandList* cl) const
{
    // 描画コマンドを設定
    cl->IASetVertexBuffers(0, 1, &vertexBufferView_);
    cl->IASetIndexBuffer(&indexBufferView_);
    
    cl->SetGraphicsRootDescriptorTable(0, skyboxTextureSrvHandleGpu_);
    cl->SetGraphicsRootConstantBufferView(1, resourceTransformationMatrix_->GetGPUVirtualAddress());
    cl->SetGraphicsRootConstantBufferView(2, resourceMaterial_->GetGPUVirtualAddress());

    cl->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void Skybox::SetSkyboxTexture(D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    skyboxTextureSrvHandleGpu_ = handle;
}

void Skybox::CreateVertices(std::array<Vector4, 24>& out_vertices)
{
    auto& vertices = out_vertices;

    // right. index : [0,1,2] [2,1,3]
    vertices[0] = { 1.0f, 1.0f, 1.0f, 1.0f };
    vertices[1] = { 1.0f, 1.0f, -1.0f, 1.0f };
    vertices[2] = { 1.0f, -1.0f, 1.0f, 1.0f };
    vertices[3] = { 1.0f, -1.0f, -1.0f, 1.0f };

    // left. index : [4,5,6] [6,5,7]
    vertices[4] = { -1.0f, 1.0f, -1.0f, 1.0f };
    vertices[5] = { -1.0f, 1.0f, 1.0f, 1.0f };
    vertices[6] = { -1.0f, -1.0f, -1.0f, 1.0f };
    vertices[7] = { -1.0f, -1.0f, 1.0f, 1.0f };

    // front. index : [8,9,10] [10,9,11]
    vertices[8] = { -1.0f, 1.0f, 1.0f, 1.0f };
    vertices[9] = { 1.0f, 1.0f, 1.0f, 1.0f };
    vertices[10] = { -1.0f, -1.0f, 1.0f, 1.0f };
    vertices[11] = { 1.0f, -1.0f, 1.0f, 1.0f };

    // back. index : [12,13,14] [14,13,15]
    vertices[12] = { 1.0f, 1.0f, -1.0f, 1.0f };
    vertices[13] = { -1.0f, 1.0f, -1.0f, 1.0f };
    vertices[14] = { 1.0f, -1.0f, -1.0f, 1.0f };
    vertices[15] = { -1.0f, -1.0f, -1.0f, 1.0f };

    // top. index : [16,17,18] [18,17,19]
    vertices[16] = { -1.0f, 1.0f, -1.0f, 1.0f };
    vertices[17] = { 1.0f, 1.0f, -1.0f, 1.0f };
    vertices[18] = { -1.0f, 1.0f, 1.0f, 1.0f };
    vertices[19] = { 1.0f, 1.0f, 1.0f, 1.0f };

    // bottom. index : [20,21,22] [22,21,23]
    vertices[20] = { 1.0f, -1.0f, -1.0f, 1.0f };
    vertices[21] = { -1.0f, -1.0f, -1.0f, 1.0f };
    vertices[22] = { 1.0f, -1.0f, 1.0f, 1.0f };
    vertices[23] = { -1.0f, -1.0f, 1.0f, 1.0f };
}

void Skybox::CreateIndices(std::array<uint32_t, 36>& out_indices)
{
    out_indices = {
        0, 1, 2, 2, 1, 3, // right
        4, 5, 6, 6, 5, 7, // left
        8, 9, 10, 10, 9, 11, // front
        12, 13, 14, 14, 13, 15, // back
        16, 17, 18, 18, 17, 19, // top
        20, 21, 22, 22, 21, 23 // bottom
    };
}

void Skybox::CreateVertexResource()
{
    auto vertices = std::array<Vector4, 24>();
    this->CreateVertices(vertices);

    /// 頂点リソースを作成
    vertexResource_ = DX12Helper::CreateBufferResource(device_, sizeof(Vector4) * vertices.size());
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertices_));

    /// 頂点データを初期化
    std::memcpy(mappedVertices_, vertices.data(), sizeof(Vector4) * vertices.size());

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<uint32_t>(sizeof(Vector4) * vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(Vector4);
}

void Skybox::CreateIndexResource()
{
    auto indices = std::array<uint32_t, 36>();
    this->CreateIndices(indices);
    const size_t sizeInBytes = sizeof(uint32_t) * indices.size();

    indexResource_ = DX12Helper::CreateBufferResource(device_, sizeInBytes);
    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertices_));

    std::memcpy(mappedVertices_, indices.data(), sizeInBytes);

    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = static_cast<uint32_t>(sizeInBytes);
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}

void Skybox::CreateTransformationMatrixResource()
{
    resourceTransformationMatrix_ = DX12Helper::CreateBufferResource(device_, sizeof(TransformationMatrix));
    resourceTransformationMatrix_->Map(0, nullptr, reinterpret_cast<void**>(&mappedTransformationMatrix_));

    transformation_.scale = Vector3(500.0f, 500.0f, 500.0f);

    mappedTransformationMatrix_->world = Matrix4x4::AffineMatrix(
        transformation_.scale,
        transformation_.rotate,
        transformation_.translate
    );
    mappedTransformationMatrix_->wvp = Matrix4x4::Identity();
}

void Skybox::CreateMaterialResource()
{
    resourceMaterial_ = DX12Helper::CreateBufferResource(device_, sizeof(Vector4));
    resourceMaterial_->Map(0, nullptr, reinterpret_cast<void**>(&mappedMaterial_));

    // Initialize material data
    *mappedMaterial_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // Default color
}

void Skybox::ImGui()
{
    #ifdef _DEBUG

    bool isChangedTransformation = false;
    ImGui::SeparatorText("Transformation");
    isChangedTransformation |= ImGui::DragFloat3("Scale", &transformation_.scale.x, 0.01f);
    isChangedTransformation |= ImGui::DragFloat3("Rotation", &transformation_.rotate.x, 0.01f);
    isChangedTransformation |= ImGui::DragFloat3("Translation", &transformation_.translate.x, 0.01f);

    if (isChangedTransformation)
    {
        mappedTransformationMatrix_->world = Matrix4x4::AffineMatrix(
            transformation_.scale,
            transformation_.rotate,
            transformation_.translate
        );
        mappedTransformationMatrix_->wvp = mappedTransformationMatrix_->world * (*ppGlobalEye_)->GetViewProjectionMatrix();
    }
    #endif
}