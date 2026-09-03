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
#include <Core/DirectX12/DirectX12.h>
#include <Vector2.h>

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

    struct TextureData
    {
        std::string filePath = {};
        DirectX::TexMetadata metadata = {};
        DX12Resource textureResource = {};
    };
    using TextureDataMap = std::unordered_map<std::filesystem::path, TextureData>;

    static TextureManager* GetInstance() { static TextureManager instance; return &instance;}
    /// <summary>
    /// テクスチャ管理を初期化します。
    /// </summary>
    /// <param name="srvManager">SRV 管理クラス。</param>
    void Initialize(SRVManager* srvManager);

    /// <summary>
    /// 中間リソース（アップロードバッファ等）を解放します。
    /// </summary>
    void ReleaseIntermediateResources();

    /// <summary>
    /// テクスチャファイルの読み込み
    /// </summary>
    /// <param name="filePath">テクスチャファイルのパス</param>
    /// <returns>実際に読み込んだファイルパス</returns>
    /// <remarks>ファイルが見つからなかった場合はデフォルトで読み込まれるテクスチャのパスを返します</remarks>
    std::string LoadTexture(const std::string& filePath);

    /// <summary>
    /// 指定されたファイルパスのテクスチャをアンロードします。
    /// </summary>
    /// <param name="filePath">アンロードするテクスチャのファイルパス。</param>
    void UnloadTexture(const std::string& filePath);

    /// <summary>
    /// 指定されたファイルパスのテクスチャを再読み込みします。
    /// </summary>
    /// <param name="filePath">再読み込みするテクスチャファイルのパス。</param>
    inline void ReloadTexture(const std::string& filePath)
    {
        UnloadTexture(filePath);
        LoadTexture(filePath);
    }


public: /// Getter
    [[nodiscard]] const DirectX::TexMetadata&   GetMetaData(const std::string& filePath);
    [[nodiscard]] const DirectX::TexMetadata&   GetMetaData(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);
    [[nodiscard]] uint32_t                      GetSrvIndex(const std::string& filePath);
    [[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE   GetSrvHandleGPU(const std::string& filePath);
    [[nodiscard]] const DX12Resource&           GetTextureResource(const std::string& filePath);
    [[nodiscard]] static Vector2                GetTextureSize(const DirectX::TexMetadata& metadata);
    [[nodiscard]] const TextureDataMap&         GetTextureDataMap() const { return textureDataMap_; }
    [[nodiscard]] TextureDataMap&               GetTextureDataMap() { return textureDataMap_; }
private:

    enum class TextureType
    {
        kUnknown,
        kWIC,      // Windows Imaging Component
        kDDS,      // DirectDraw Surface
    };

    /// <summary>
    /// ファイル拡張子からテクスチャ種別を判定します。
    /// </summary>
    /// <param name="_filePath">ファイルパス。</param>
    /// <returns>テクスチャ種別。</returns>
    TextureType GetTextureType(const std::wstring& _filePath) const;
    
    /// <summary>
    /// 画像ファイルを読み込みます。
    /// </summary>
    /// <param name="_type">テクスチャ種別。</param>
    /// <param name="_filepath">ファイルパス。</param>
    /// <param name="_image">読み込まれた画像データの出力先。</param>
    /// <returns>HRESULT。</returns>
    HRESULT LoadImageFromFile(TextureType _type, const std::wstring& _filepath, DirectX::ScratchImage& _image);
    
    /// <summary>
    /// SRV を作成します。
    /// </summary>
    /// <param name="_type">テクスチャ種別。</param>
    /// <param name="_textureData">テクスチャデータ。</param>
    void CreateSRV(const TextureData& _textureData);
    
    /// <summary>
    /// 検索パスに基づきファイルパスを解決します。
    /// </summary>
    /// <param name="_filePath">元のファイルパス。</param>
    /// <returns>解決済みファイルパス。</returns>
    std::string ResolveFilePath(const std::string& _filePath);

    TextureDataMap textureDataMap_;
    PathResolver pathResolver_ = {};
    std::list<Microsoft::WRL::ComPtr<ID3D12Resource>> resourcesIntermediate_;

    // Pointers
    SRVManager* srvManager_ = nullptr;

private:
    TextureManager() = default;
    ~TextureManager() = default;
};