// Copyright © 2024 Souto-Naitou. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.


#ifndef CSVLOADER_H_
#define CSVLOADER_H_

#include "csvStructure/CSVData.h"

#include <utility>
#include <string>
#include <list>

/// <summary>
/// CSVローダークラス
/// </summary>
class CSVLoader{
public:
    CSVLoader(const CSVLoader&) = delete;
    CSVLoader& operator=(const CSVLoader&) = delete;
    CSVLoader(CSVLoader&&) = delete;
    CSVLoader& operator=(const CSVLoader&&) = delete;

    /// <summary>
    /// インスタンスを取得
    /// </summary>
    /// <returns>インスタンス</returns>
    static CSVLoader* GetInstance() { static CSVLoader instance; return &instance; }

    /// <summary>
    /// データを取得
    /// </summary>
    /// <returns></returns>
    std::list<std::pair<std::string, CSVData>>* GetData() { return &fileData_; }

    /// <summary>
    /// ファイルを読み込む
    /// </summary>
    /// <param name="_fileName">ファイル名。oo.csv</param>
    /// <returns>読んだデータ</returns>
    CSVData* LoadFile(std::string _fileName);

    /// <summary>
    /// 指定ファイルの指定ID行を取得します。
    /// </summary>
    /// <param name="_fileName">CSVファイル名。</param>
    /// <param name="_ID">行ID。</param>
    /// <returns>該当行。なければ nullptr。</returns>
    CSVLine* GetLine(std::string _fileName, std::string _ID);
    /// <summary>
    /// 指定ファイルに新規行を追加し、その行を取得します。
    /// </summary>
    /// <param name="_fileName">CSVファイル名。</param>
    /// <returns>新規行。</returns>
    CSVLine* GetNewLine(std::string _fileName);

    /// <summary>
    /// 変更内容を全てのファイルに保存します。
    /// </summary>
    void SaveFile();
    /// <summary>
    /// 1行分の変更をファイルへ保存します。
    /// </summary>
    /// <param name="_fileName">CSVファイル名。</param>
    /// <param name="_line">保存する行データ。</param>
    void SaveLine(std::string _fileName, CSVLine& _line);
    void SetDirectory(std::string _dir) { directory_ = _dir; }

private:
    CSVLoader();
    ~CSVLoader();

    std::string directory_ = "Resources/CSV/";

    std::list<std::pair<std::string, CSVData>> fileData_;

    std::string previewFile_;
private:
    /// <summary>
    /// デバッグウィンドウを描画します。
    /// </summary>
    void DebugWindow();
};


#endif // CSVLOADER_H_