#pragma once
#include <any>
#include <string>

class ISceneArgs
{
public:
    virtual ~ISceneArgs() {}
    
    /// <summary>
    /// 指定キーの引数を取得します。
    /// </summary>
    /// <param name="key">引数キー。</param>
    /// <returns>値（書き換え可能）。</returns>
    virtual std::any& Get(const std::string& key) = 0;
    
    /// <summary>
    /// 指定キーの引数を取得します（読み取り専用）。
    /// </summary>
    /// <param name="key">引数キー。</param>
    /// <returns>値（読み取り専用）。</returns>
    virtual const std::any& Get(const std::string& key) const = 0;
    
    /// <summary>
    /// 指定キーに値を設定します。
    /// </summary>
    /// <param name="key">引数キー。</param>
    /// <param name="value">設定する値。</param>
    virtual void Set(const std::string& key, const std::any& value) = 0;
    
    /// <summary>
    /// 登録済みの引数をすべてクリアします。
    /// </summary>
    virtual void Reset() = 0;
    
    /// <summary>
    /// 指定キーの引数を削除します。
    /// </summary>
    /// <param name="key">削除する引数のキー。</param>
    virtual void Erase(const std::string& key) = 0;
};