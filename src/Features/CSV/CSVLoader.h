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

    CSVLine* GetLine(std::string _fileName, std::string _ID);
    CSVLine* GetNewLine(std::string _fileName);

    void SaveFile();
    void SaveLine(std::string _fileName, CSVLine& _line);
    void SetDirectory(std::string _dir) { directory_ = _dir; }

private:
    CSVLoader();
    ~CSVLoader();

    std::string directory_ = "Resources/CSV/";

    std::list<std::pair<std::string, CSVData>> fileData_;

    std::string previewFile_;
private:
    void DebugWindow();
};


#endif // CSVLOADER_H_