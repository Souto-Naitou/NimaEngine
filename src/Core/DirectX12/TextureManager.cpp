#include "TextureManager.h"
#include "DirectX12.h"
#include <Core/DirectX12/Helper/DX12Helper.h>
#include <Utility/ConvertString/ConvertString.h>
#include <Core/ConfigManager/ConfigManager.h>

void TextureManager::Initialize(SRVManager* _srvManager)
{
    srvManager_ = _srvManager;
    textureDataMap_.reserve(DirectX12::kMaxSRVCount_);

    auto& cfgData = ConfigManager::GetInstance()->GetConfigData();
    for (auto& path : cfgData.texture_paths)
    {
        this->AddSearchPath(path);
    }
}

void TextureManager::ReleaseIntermediateResources()
{
    resourcesIntermediate_.clear();
}

void TextureManager::LoadTexture(const std::string& _filePath)
{
    std::string fullPath = pathResolver_.GetFilePath(_filePath);

    /// すでに読み込まれている場合は読み込まない
    for (auto& [key, value] : textureDataMap_)
    {
        if (std::filesystem::equivalent(key, std::filesystem::path(fullPath)))
        {
            return;
        }
    }

    assert(!srvManager_->IsFull() && "SRVがいっぱいです");

    TextureData& textureData = textureDataMap_[fullPath];

    // ファイルから画像を読み込む
    DirectX::ScratchImage image{};
    std::wstring filePathW = ConvertString(fullPath);
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

    textureData.metadata = metadata;
    auto tempResource = DX12Helper::CreateTextureResource(pDx12_->GetDevice(), textureData.metadata);
    textureData.textureResource.Initialize(
        tempResource,
        D3D12_RESOURCE_STATE_COPY_DEST,
        textureData.metadata.format,
        fullPath
    );
    
    resourcesIntermediate_.push_back(
        DX12Helper::UploadTextureData(
            textureData.textureResource.GetResource(),
            image, 
            pDevice,
            cl
        )
    );

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
}

void TextureManager::UnloadTexture(const std::string& _filePath)
{
    std::string fullPath = pathResolver_.GetFilePath(_filePath);
    auto it = textureDataMap_.find(fullPath);
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

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& _filePath)
{
    std::string fullPath = pathResolver_.GetFilePath(_filePath);
    std::filesystem::path fsFull(fullPath);

    for (auto& [key, value] : textureDataMap_)
    {
        if (std::filesystem::equivalent(key, fsFull))
        {
            return value.textureResource.GetSRVHandleGPU();
        }
    }

    assert(false && "テクスチャが見つかりません");
    return {};
}

const DX12Resource& TextureManager::GetTextureResource(const std::string& _filePath)
{
    std::string fullPath = pathResolver_.GetFilePath(_filePath);
    const TextureData& textureData = textureDataMap_[fullPath];
    return textureData.textureResource;
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

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& _filePath)
{
    std::string fullPath = pathResolver_.GetFilePath(_filePath);
    const TextureData& textureData = textureDataMap_[fullPath];
    return textureData.metadata;
}

uint32_t TextureManager::GetSrvIndex(const std::string& _filePath)
{
    std::string fullPath = pathResolver_.GetFilePath(_filePath);
    const TextureData& textureData = textureDataMap_[fullPath];
    return textureData.textureResource.GetSRVIndex();
}
