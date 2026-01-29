#include "TextureManager.h"
#include "DirectX12.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Utility/ConvertString/ConvertString.h>
#include <Core/ConfigManager/ConfigManager.h>

void TextureManager::Initialize(SRVManager* srvManager)
{
    srvManager_ = srvManager;
    textureDataMap_.reserve(DirectX12::kMaxSRVCount_);

    auto& cfgData = ConfigManager::GetInstance()->GetConfigData();
    for (auto& path : cfgData.texture_paths)
    {
        pathResolver_.AddSearchPath(path);
    }
}

void TextureManager::ReleaseIntermediateResources()
{
    resourcesIntermediate_.clear();
}

std::string TextureManager::LoadTexture(const std::string& filePath)
{
    std::string strPathResolved = this->ResolveFilePath(filePath);

    /// すでに読み込まれている場合は読み込まない
    for (auto& [key, value] : textureDataMap_)
    {
        if (std::filesystem::equivalent(key, std::filesystem::path(strPathResolved)))
        {
            return filePath;
        }
    }

    assert(!srvManager_->IsFull() && "SRVがいっぱいです");

    TextureData& textureData = textureDataMap_[strPathResolved];

    // ファイルから画像を読み込む
    DirectX::ScratchImage image{};
    std::wstring filePathW = ConvertString(strPathResolved);
    TextureType textureType = this->GetTextureType(filePathW);
    HRESULT hr = this->LoadImageFromFile(textureType, filePathW, image);

    assert(SUCCEEDED(hr));

    DirectX::TexMetadata metadata = image.GetMetadata();

    if (DirectX::IsCompressed(metadata.format)) {
        DirectX::ScratchImage decompressed;
        DirectX::Decompress(image.GetImages(), image.GetImageCount(), metadata, DXGI_FORMAT_R8G8B8A8_UNORM, decompressed);
        image = std::move(decompressed);
    }

    if (metadata.width > 32 && metadata.height > 32)
    {
        DirectX::ScratchImage mipChain = {};
        hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 4, mipChain);
        image = std::move(mipChain);
        metadata = image.GetMetadata();
        assert(SUCCEEDED(hr));
    }

    auto pDevice = pDx12_->GetDevice();
    auto cl = pDx12_->GetCommandList();

    if (textureType == TextureType::kDDS)
    {
        // キューブマップの場合はフラグを設定
        metadata.miscFlags |= DirectX::TEX_MISC_TEXTURECUBE;
    }
    textureData.metadata = metadata;

    auto tempResource = DX12Helper::CreateTextureResource(pDx12_->GetDevice(), textureData.metadata);

    DX12Resource::Params params{};
    params.format = metadata.format;
    params.name = strPathResolved;
    params.resource = tempResource;
    params.state = D3D12_RESOURCE_STATE_COPY_DEST;
    params.pRTVCounter = pDx12_->GetRTVHeapCounter();

    textureData.textureResource.Initialize(params);

    // アップロード用の中間リソースを作成してデータ転送
    resourcesIntermediate_.push_back(
        DX12Helper::UploadTextureData(
            textureData.textureResource.GetResource(),
            image, 
            pDevice,
            cl
        )
    );

    // SRV用のデスクリプタを確保
    uint32_t srvIndex = srvManager_->Allocate();
    auto srvHandleCPU = srvManager_->GetCPUDescriptorHandle(srvIndex);
    auto srvHandleGPU = srvManager_->GetGPUDescriptorHandle(srvIndex);

    textureData.textureResource.SetSRV(
        srvIndex,
        srvHandleCPU,
        srvHandleGPU
    );

    // Typeに応じてSRVを作成
    this->CreateSRV(textureType, textureData);

    return strPathResolved;
}

void TextureManager::UnloadTexture(const std::string& filePath)
{
    std::string strPathResolved = this->ResolveFilePath(filePath);
    auto it = textureDataMap_.find(strPathResolved);
    if (it == textureDataMap_.end())
    {
        return; // テクスチャが見つからない場合は何もしない
    }
    const TextureData& textureData = it->second;
    // SRVを解放
    srvManager_->Deallocate(textureData.textureResource.GetSRVIndex());
    // リソースを解放
    if (textureData.textureResource.GetResource())
    {
        resourcesIntermediate_.remove(textureData.textureResource.GetResource());
    }
    textureDataMap_.erase(it); // マップから削除
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
    std::string strPathResolved = this->ResolveFilePath(filePath);
    std::filesystem::path fsPathResolved(strPathResolved);

    for (auto& [key, value] : textureDataMap_)
    {
        if (std::filesystem::equivalent(key, fsPathResolved))
        {
            return value.textureResource.GetSRVHandleGPU();
        }
    }

    assert(false && "テクスチャが見つかりません");
    return {};
}

const DX12Resource& TextureManager::GetTextureResource(const std::string& _filePath)
{
    std::string resolvedPath = this->ResolveFilePath(_filePath);
    const TextureData& textureData = textureDataMap_[resolvedPath];
    return textureData.textureResource;
}

Vector2 TextureManager::GetTextureSize(const DirectX::TexMetadata& metadata)
{
    return Vector2(static_cast<float>(metadata.width), static_cast<float>(metadata.height));
}

TextureManager::TextureType TextureManager::GetTextureType(const std::wstring& _filePath) const
{
    if (_filePath.ends_with(L".dds"))
    {
        return TextureType::kDDS;
    }
    else if (_filePath.ends_with(L".png") || _filePath.ends_with(L".jpg") || _filePath.ends_with(L".jpeg"))
    {
        return TextureType::kWIC;
    }
    else
    {
        return TextureType::kUnknown;
    }
}

HRESULT TextureManager::LoadImageFromFile(TextureType _type, const std::wstring& _filepath, DirectX::ScratchImage& _image)
{
    switch (_type)
    {
        case TextureType::kDDS:
            return DirectX::LoadFromDDSFile(_filepath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, _image);
        case TextureType::kWIC:
            return DirectX::LoadFromWICFile(_filepath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, _image);
    }
    return E_FAIL; // Unknown type
}

void TextureManager::CreateSRV(TextureType _type, const TextureData& _textureData)
{
    switch (_type)
    {
        case TextureType::kDDS:
            srvManager_->CreateForCubemap(
                _textureData.textureResource.GetSRVIndex(),
                _textureData.textureResource.GetResource().Get(),
                _textureData.metadata.format,
                UINT32_MAX
            );
            break;
        case TextureType::kWIC:
            srvManager_->CreateForTexture2D(
                _textureData.textureResource.GetSRVIndex(),
                _textureData.textureResource.GetResource().Get(),
                _textureData.metadata.format,
                static_cast<UINT>(_textureData.metadata.mipLevels)
            );
            break;
        default:
            assert(false && "Unsupported texture type");
            break;
    }
}

std::string TextureManager::ResolveFilePath(const std::string& filePath)
{
    std::string result = pathResolver_.GetFilePath(filePath);
    if (result.empty())
    {
        Logger::GetInstance()->LogError(__FILE__, __FUNCTION__, "Missing file [" + filePath + "]");
        // 見つからなかった場合はデフォルトテクスチャを返す
        result = pathResolver_.GetFilePath("white1x1.png");
    }

    return result;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
    std::string resolvedPath = this->ResolveFilePath(filePath);
    const TextureData& textureData = textureDataMap_[resolvedPath];
    return textureData.metadata;
}

const DirectX::TexMetadata& TextureManager::GetMetaData(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle)
{
    for (const auto& [key, value] : textureDataMap_)
    {
        if (value.textureResource.GetSRVHandleGPU().ptr == gpuHandle.ptr)
        {
            return value.metadata;
        }
    }
    assert(false && "テクスチャが見つかりません");
    static DirectX::TexMetadata emptyMetadata{};
    return emptyMetadata;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath)
{
    std::string resolvedPath = this->ResolveFilePath(filePath);
    const TextureData& textureData = textureDataMap_[resolvedPath];
    return textureData.textureResource.GetSRVIndex();
}
