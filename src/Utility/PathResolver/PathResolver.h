#pragma once

#include <string>
#include <filesystem>
#include <vector>

class PathResolver
{
public:
    PathResolver() = default;
    ~PathResolver() = default;

    /// <summary>
    /// パス解決に必要な初期化を行います。
    /// </summary>
    void Initialize();
    /// <summary>
    /// リソースを解放します。
    /// </summary>
    void Finalize();

    /// <summary>
    /// 検索パスを追加
    /// </summary>
    /// <param name="path">パス</param>
    void AddSearchPath(const std::string& path);


    /// <summary>
    /// ファイルの親ディレクトリを取得
    /// </summary>
    /// <returns>親ディレクトリのパス</returns>
    [[nodiscard]]
    std::string GetParentPath(const std::string& fileName);


    /// <summary>
    /// 検索パスから検索し、ファイルパスを取得
    /// </summary>
    /// <param name="fileName">ファイル名</param>
    /// <returns>ファイルパス</returns>
    [[nodiscard]]
    std::string GetFilePath(const std::string& fileName);

private:
    std::vector<std::filesystem::path> searchPaths_ = {};
};