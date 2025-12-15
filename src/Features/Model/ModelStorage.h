#pragma once
#include <unordered_map>
#include <filesystem>
#include <memory>

#include "./IModel.h"

// 読み込み済みモデル保持クラス
// - ファイルから一度読み込んだモデルのデータを保持するクラス
class ModelStorage
{
public:
    /// <summary>
    /// 保持しているモデルを解放します。
    /// </summary>
    void Finalize();

    /// <summary>
    /// モデルデータを取得
    /// </summary>
    /// <param name="filePath">ファイルパス</param>
    /// <returns>モデルデータ</returns>
    IModel* FindModel(const std::string& path);

    /// <summary>
    /// 指定パスが既に読み込まれているか判定します。
    /// </summary>
    /// <param name="path">ファイルパス。</param>
    /// <returns>読み込み済みなら true。</returns>
    bool IsLoaded(const std::filesystem::path& path);

    /// <summary>
    /// モデルをストレージに追加し、参照を返します。
    /// </summary>
    /// <param name="path">キーとなるファイルパス。</param>
    /// <param name="model">追加するモデル（共有所有）。</param>
    /// <returns>追加されたモデルの生ポインタ。</returns>
    IModel* AddModel(const std::filesystem::path& path, std::shared_ptr<IModel> model);

private:
    std::unordered_map<std::filesystem::path, std::shared_ptr<IModel>> models_;
};