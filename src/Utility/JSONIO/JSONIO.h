#pragma once

#include <nlohmann/json.hpp>
#include <utility>

class JsonLoader;

class JSONStorage
{
    friend class JSONIO;


private:
    JSONStorage() = default;

    using json = nlohmann::json;
    std::unordered_map<std::string, json> jsonDataMap_;


public:
    class Iterator;


public:
    /// <summary>
    /// キーに対応する JSON への参照を取得（なければ生成）します。
    /// </summary>
    /// <param name="_key">キー。</param>
    /// <returns>JSON 参照。</returns>
    json& operator[](const std::string& _key);
    
    /// <summary>
    /// 先頭イテレータを取得します。
    /// </summary>
    Iterator begin();
    
    /// <summary>
    /// 終端イテレータを取得します。
    /// </summary>
    Iterator end();
    
    /// <summary>
    /// 指定キーの要素を削除します。
    /// </summary>
    /// <param name="_key">キー。</param>
    /// <returns>削除件数。</returns>
    size_t erase(const std::string& _key);

public:
    /// <summary>
    /// 指定キーの JSON 参照を取得します（存在しない場合は例外）。
    /// </summary>
    /// <param name="_key">キー。</param>
    /// <returns>JSON 参照。</returns>
    json& at(const std::string& _key);
    
    /// <summary>
    /// 指定キーのイテレータを取得します。
    /// </summary>
    /// <param name="_key">キー。</param>
    /// <returns>該当要素のイテレータ、見つからなければ end()。</returns>
    Iterator find(const std::string& _key);


private:
    std::string ToAbsPathLower(const std::string& _filepath) const;
};

class JSONStorage::Iterator
{
private:
    using jsonmap = std::unordered_map<std::string, json>;
    jsonmap::iterator it_;

public:
    Iterator() = default;
    Iterator(jsonmap::iterator _it) : it_(_it) {}


    std::pair<const std::string, json> operator*() const;
    Iterator& operator++();
    Iterator& operator--();
    bool operator==(const Iterator& _other) const;
    bool operator!=(const Iterator& _other) const;

};

class JSONIO
{
private:
    JSONIO() = default;
    ~JSONIO() = default;

    using json = nlohmann::json;
    JSONStorage jsonDataStorage_;

public:
    JSONIO(const JSONIO&) = delete;
    JSONIO& operator=(const JSONIO&) = delete;
    JSONIO(JSONIO&&) = delete;
    JSONIO& operator=(JSONIO&&) = delete;

    static JSONIO* GetInstance()
    {
        static JSONIO instance;
        return &instance;
    }


    /// <summary>
    /// JSON ファイルを読み込み、ストレージにキャッシュします。
    /// </summary>
    /// <param name="_path">ファイルパス。</param>
    /// <returns>読み込んだ JSON 参照。</returns>
    const json& Load(const std::string& _path);
    
    /// <summary>
    /// JSON データをファイルへ保存します。
    /// </summary>
    /// <param name="_path">保存先ファイルパス。</param>
    /// <param name="_jsonData">保存する JSON。</param>
    void Save(const std::string& _path, const json& _jsonData);
    
    /// <summary>
    /// 指定パスの JSON をストレージからアンロードします。
    /// </summary>
    /// <param name="_path">ファイルパス。</param>
    /// <returns>アンロードできた場合は true。</returns>
    bool Unload(const std::string& _path);
};