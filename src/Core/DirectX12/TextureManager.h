#pragma once

#include <string>
#include <DirectXTex.h>
#include <wrl.h>
#include <d3d12.h>
#include <Core/DirectX12/SRVManager.h>
#include <unordered_map>
#include <Utility/PathResolver/PathResolver.h>
#include <Core/DirectX12/DX12Resource/DX12Resource.h>
#include <list>
#include <wrl/client.h>
#include <filesystem>

/// <summary>
/// テクスチャ管理クラス
/// </summary>
class TextureManager : public EngineFeature
{
public:
    TextureManager(const TextureManager&) = delete;
    TextureManager(const TextureManager&&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&&) = delete;

    static TextureManager* GetInstance() { static TextureManager instance; return &instance;}

    void Initialize(SRVManager* _srvManager);

    void ReleaseIntermediateResources();

    /// <summary>
    /// テクスチャファイルの読み込み
    /// </summary>
    /// <param name="_filePath">テクスチャファイルのパス</param>
    void LoadTexture(const std::string& _filePath);

    /// <summary>
    /// 指定されたファイルパスのテクスチャをアンロードします。
    /// </summary>
    /// <param name="_filePath">アンロードするテクスチャのファイルパス。</param>
    void UnloadTexture(const std::string& _filePath);

    /// <summary>
    /// 指定されたファイルパスのテクスチャを再読み込みします。
    /// </summary>
    /// <param name="_filePath">再読み込みするテクスチャファイルのパス。</param>
    inline void ReloadTexture(const std::string& _filePath)
    {
        UnloadTexture(_filePath);
        LoadTexture(_filePath);
    }


public: /// Setter
    void AddSearchPath(const std::string& _path) { pathResolver_.AddSearchPath(_path); }


public: /// Getter
    [[nodiscard]] const DirectX::TexMetadata&   GetMetaData(const std::string& _filePath);
    [[nodiscard]] uint32_t                      GetSrvIndex(const std::string& _filePath);
    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE   GetSrvHandleGPU(const std::string& _filePath);
    [[nodiscard]] const DX12Resource&        GetTextureResource(const std::string& _filePath);

private:
    struct TextureData
    {
        DirectX::TexMetadata metadata = {};
        DX12Resource textureResource = {};
    };

    enum class TextureType
    {
        kUnknown,
        kWIC,      // Windows Imaging Component
        kDDS,      // DirectDraw Surface
    };

    TextureType GetTextureType(const std::wstring& _filePath) const;
    HRESULT LoadImageFromFile(TextureType _type, const std::wstring& _filepath, DirectX::ScratchImage& _image);
    void CreateSRV(TextureType _type, const TextureData& _textureData);
    std::string ResolveFilePath(const std::string& _filePath);

    std::unordered_map<std::filesystem::path, TextureData> textureDataMap_;
    PathResolver pathResolver_ = {};
    std::list<Microsoft::WRL::ComPtr<ID3D12Resource>> resourcesIntermediate_;

    // Pointers
    SRVManager* srvManager_ = nullptr;

private:
    TextureManager() = default;
    ~TextureManager() = default;

};