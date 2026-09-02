#include "Sprite.h"

#include "SpriteSystem.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Matrix4x4.h>
#include <Core/DirectX12/TextureManager.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <Common/define.h>
#include <Features/Layer/Canvas.h>

#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG
#include <Core/Window/Window.h>
#include <cmath>

Sprite::Sprite()
{
}


Sprite::~Sprite()
{

}


void Sprite::Initialize(std::string filepath)
{
    texturePath_ = TextureManager::GetInstance()->LoadTexture(filepath);

    textureSrvHandleGPU_ = TextureManager::GetInstance()->GetSrvHandleGPU(texturePath_);

    Initialize(textureSrvHandleGPU_);
}


void Sprite::Initialize(D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
    pSpriteSystem_ = SpriteSystem::GetInstance();
    pDx12_ = pSpriteSystem_->GetDirectX12();
    device_ = pDx12_->GetDevice();
    textureSrvHandleGPU_ = handle;

    // デバッグウィンドウの登録
    pDebugEntry_ = std::make_unique<DebugEntry<Sprite>>("Sprite", "unnamed", this, false);

    /// Create BufferResource
    // 頂点リソースを作成する
    CreateVertexResource();

    // インデックスリソースを作成する
    CreateIndexResource();


    /// Create View
    // 頂点バッファビューを作成する
    CreateVertexBufferView();

    // インデックスバッファビューを作成する
    CreateIndexBufferView();


    /// Map
    // 頂点データを設定する
    MapVertexData();

    // インデックスデータを設定する
    MapIndexData();

    /// Material
    // マテリアルリソースを作成する
    CreateMaterialResource();

    // 座標変換行列を表すデータを作成する
    CreateTransformationMatrixResource();

    metadata_ = &this->GetMetadata();

    InitializeSize();
}

void Sprite::Update()
{
    if (!isUpdateEnabled_) return;
    #ifdef _DEBUG
    isUpdateCalled_ = true;
    #endif // _DEBUG

    uint32_t clientWidth = Window::clientWidth;
    uint32_t clientHeight = Window::clientHeight;

    this->UpdateMetadata();

    // 左下
    vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };
    // 左上
    vertexData_[1].normal = { 0.0f, 0.0f, -1.0f };
    // 右下
    vertexData_[2].normal = { 0.0f, 0.0f, -1.0f };
    // 右上
    vertexData_[3].normal = { 0.0f, 0.0f, -1.0f };

    /// アンカーポイントの設定と適用
    float left      = 0.0f - anchorPoint_.x;
    float right     = 1.0f - anchorPoint_.x;
    float top       = 0.0f - anchorPoint_.y;
    float bottom    = 1.0f - anchorPoint_.y;

    if (isFlipX_)
    {
        left = -left;
        right = -right;
    }

    if (isFlipY_)
    {
        top = -top;
        bottom = -bottom;
    }

    vertexData_[0].position = { left, bottom, 0.0f, 1.0f };
    vertexData_[1].position = { left, top, 0.0f, 1.0f };
    vertexData_[2].position = { right, bottom, 0.0f, 1.0f };
    vertexData_[3].position = { right, top, 0.0f, 1.0f };

    /// UV座標の設定と適用 (範囲指定)
    float tex_left = textureLeftTop_.x / metadata_->width;
    float tex_top = textureLeftTop_.y / metadata_->height;
    float tex_right = (textureLeftTop_.x + textureSize_.x) / metadata_->width;
    float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metadata_->height;

    vertexData_[0].texcoord = { tex_left, tex_bottom };
    vertexData_[1].texcoord = { tex_left, tex_top };
    vertexData_[2].texcoord = { tex_right, tex_bottom };
    vertexData_[3].texcoord = { tex_right, tex_top };

    /// トランスフォームの更新
    transform_.scale = { size_.x, size_.y, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, rotate_ };
    transform_.translate = translate_;

    worldMatrix_ = Matrix4x4::AffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    /// UVTransformMatrixの更新 (まだ使えない)
    //uvTransformMatrix_ = Matrix4x4::ScaleMatrix(uvTransform_.scale);
    //uvTransformMatrix_ *= Matrix4x4::RotateZMatrix(uvTransform_.rotate.z);
    //uvTransformMatrix_ *= Matrix4x4::TranslateMatrix(uvTransform_.translate);
}

void Sprite::DrawCall(ID3D12GraphicsCommandList* cl)
{
    if (!isDrawEnabled_) return;

    /// VP行列を適用する
    auto pCurrentCanvas = this->GetCurrentCanvas();
    auto pGameEye = pCurrentCanvas ? pCurrentCanvas->GetGameEye() : nullptr;

    Matrix4x4 vp = {};
    Matrix4x4 world = {};
    if (!pGameEye)
    {
        vp = Matrix4x4::OrthographicMatrix(0.0f, 0.0f, float(Window::clientWidth), float(Window::clientHeight), -1.0f, 1.0f);
        world = worldMatrix_;
    }
    else if (pGameEye->IsOrthographic2d())
    {
        vp = pGameEye->GetViewProjectionMatrix();
        world = Matrix4x4::ScaleMatrix({ 1,-1,1 }) * worldMatrix_;
    }
    else
    {
        vp = pGameEye->GetViewProjectionMatrix();
        world = worldMatrix_;
    }

    transformationMatrixData_->world = world;
    transformationMatrixData_->wvp = world * vp;

    SpriteSystem::CommandListData data{};
    data.materialResource = materialResource_.Get();
    data.transformationMatrixResource = transformationMatrixResource_.Get();
    data.srvHandleGPU = textureSrvHandleGPU_;
    data.pVBV = &vertexBufferView_;
    data.pIBV = &indexBufferView_;
    data.rtvHandleCPU = DrawableBase::GetRTVHandleCPU();;

    SpriteSystem::GetInstance()->DrawSingle(cl, data);
}

void Sprite::Finalize()
{
}

void Sprite::SetSizeWithFactor(float factor)
{
    size_ = Vector2(
        static_cast<float>(metadata_->width), 
        static_cast<float>(metadata_->height)
    ) * factor;
}

/// 頂点リソースを作成する
void Sprite::CreateVertexResource()
{
    vertexResource_ = DX12Helper::CreateBufferResource(device_, sizeof(VertexData) * 4);
}


/// インデックスリソースを作成
void Sprite::CreateIndexResource()
{
    indexResource_ = DX12Helper::CreateBufferResource(device_, sizeof(uint32_t) * 6);
}


/// 頂点バッファビューを作成する
void Sprite::CreateVertexBufferView()
{
    // リソースの先頭のアドレスから使う
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    // 使用するリソースのサイズは頂点６つ分のサイズ
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    // 1頂点あたりのサイズ
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
}


/// IBVの作成
void Sprite::CreateIndexBufferView()
{
    // リソースの先頭のアドレスから使用する
    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    // 使用するリソースのサイズはインデックス6つ分のサイズ
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    // インデックスはint32_tとする
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
}


/// 頂点データを設定する
void Sprite::MapVertexData()
{
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
}


/// インデックスデータを設定する
void Sprite::MapIndexData()
{
    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
    indexData_[0] = 0;	indexData_[1] = 1;	indexData_[2] = 2;
    indexData_[3] = 1; indexData_[4] = 3; indexData_[5] = 2;
}


/// マテリアルリソースを作成する
void Sprite::CreateMaterialResource()
{
    materialResource_ = DX12Helper::CreateBufferResource(device_, sizeof(Material));

    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

    materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData_->uvTransform = Matrix4x4::Identity();
}


/// TransformMatrix用のリソースを作成する
void Sprite::CreateTransformationMatrixResource()
{
    transformationMatrixResource_ = DX12Helper::CreateBufferResource(device_, sizeof(TransformationMatrix)); // TransformationMatrix
    // データを書き込む
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    // 単位行列を書き込んでおく
    transformationMatrixData_->wvp = Matrix4x4::Identity();
    transformationMatrixData_->world = Matrix4x4::Identity();
}

void Sprite::InitializeSize()
{
    // テクスチャのサイズを取得
    auto textureWidth = static_cast<float>(metadata_->width);
    auto textureHeight = static_cast<float>(metadata_->height);

    // サイズを調整
    size_ = Vector2(textureWidth, textureHeight);
    textureLeftTop_ = Vector2(0.0f, 0.0f);
    textureSize_ = Vector2(textureWidth, textureHeight);

    aspectRatio_ = size_.x / size_.y;
}

const DirectX::TexMetadata& Sprite::GetMetadata() const
{
    return TextureManager::GetInstance()->GetMetaData(textureSrvHandleGPU_);
}

void Sprite::ImGui()
{
#if defined _DEBUG

    thumbnailSize_.x = ImGui::GetContentRegionAvail().x - 16;
    thumbnailSize_.y = thumbnailSize_.x / aspectRatio_;

    ImGui::BeginChild("Preview", ImVec2(-1, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - thumbnailSize_.x) * 0.5f, ImGui::GetCursorPosY() + (ImGui::GetContentRegionAvail().y - thumbnailSize_.y) * 0.5f));
    ImGui::Image(static_cast<ImTextureID>(textureSrvHandleGPU_.ptr), ImVec2(thumbnailSize_.x, thumbnailSize_.y));
    ImGui::EndChild();

    ImGui::Checkbox("Update", &isUpdateEnabled_);
    ImGui::SameLine();
    ImGui::Checkbox("Draw2D", &isDrawEnabled_);

    // FIX: 複数インスタンスがImGui()を呼び出すとtの値が加速する
    static float t = 1.0f;
    if (!isUpdateCalled_)
    {
        ImGui::SameLine();
        float opacity = (std::sinf(t) + 1.0f) / 2.0f;
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, opacity), "Update is not called!");
        t += 0.04f;
    }

    ImGui::SeparatorText("Transform");
    ImGui::DragFloat2("Size", &size_.x, 0.1f);
    ImGui::DragFloat("Rotate", &rotate_, 0.1f);
    ImGui::DragFloat2("Translate", &translate_.x, 0.1f);

    ImGui::SeparatorText("Option");
    ImGui::DragFloat2("AnchorPoint", &anchorPoint_.x, 0.1f);
    ImGui::Checkbox("FlipX", &isFlipX_);
    ImGui::Checkbox("FlipY", &isFlipY_);

    isUpdateCalled_ = false;
#endif // _DEBUG && DEBUG_ENGINE
}

void Sprite::UpdateMetadata()
{
    metadata_ = &this->GetMetadata();
}
