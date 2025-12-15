#pragma once

#include <Core/DirectX12/DirectX12.h>
#include <Features/Model/Loader/IModelLoader.h>
#include <Features/Model/ModelStorage.h> // ModelStorage(Concrete)
#include <Utility/PathResolver/PathResolver.h>

class Particle;

/// モデル管理クラス
/// - ModelLoaderとModelStorageを統括するクラス
class ModelManager
{
public:
    ModelManager() = default;
    ~ModelManager() = default;
    /// <summary>
    /// モデル管理の初期化を行います。
    /// パスリゾルバやストレージの初期設定を行います。
    /// </summary>
    void Initialize();

    void SetModelLoader(IModelLoader* loader);
    void SetModelStorage(ModelStorage* storage);

    /// <summary>
    /// 事前ロードの対象パスを追加します。
    /// </summary>
    /// <param name="path">モデルファイルまたはディレクトリのパス。</param>
    void AddLoadPath(const std::string& path); 
    /// <summary>
    /// 検索対象のパスを追加します。
    /// </summary>
    /// <param name="path">検索ディレクトリ。</param>
    void AddSearchPath(const std::string& path);
    /// <summary>
    /// 登録済みのロードパスを全て読み込みます。
    /// </summary>
    /// <returns>読み込んだ件数。</returns>
    int LoadAll();

    /// <summary>
    /// 単一のモデルを読み込みます。
    /// </summary>
    /// <param name="path">モデルファイルのパス。</param>
    /// <returns>読み込んだモデルのポインタ。失敗時は nullptr。</returns>
    IModel* Load(const std::string& path);

private:
    IModelLoader*           pModelLoader_   = nullptr;
    ModelStorage*           pModelStorage_  = nullptr;
    PathResolver            pathResolver_   = {};
    std::list<std::string>  loadPaths_      = {};
};