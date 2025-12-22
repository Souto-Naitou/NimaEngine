// Copyright © 2024 Souto-Naitou. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.


#include "CSVLoader.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <Windows.h>
#include <imgui.h>
#include <DebugTools/DebugManager/DebugManager.h>
#include <functional> // std::function を使用するために必要

CSVLoader::CSVLoader()
{
    DebugManager::GetInstance()->SetComponent("[Window]", "CSVLoader", std::bind(&CSVLoader::DebugWindow, this));
}

CSVData* CSVLoader::LoadFile(std::string fileName)
{
    /// ファイルがすでにロードされていたらりーたん
    for (auto& file : fileData_)
    {
        if (file.first.compare(fileName) == 0) // 0のときは合致
        {
            return &file.second;
        }
    }

    /// ファイルを読み込む
    std::stringstream fileStream;
    std::ifstream file;
    std::string path = directory_ + fileName;
    file.open(path);
    if (!file.is_open())
    {
        std::ofstream ofile(path, std::ios::out);
        ofile.close();
        file.open(path);
    }
    fileStream << file.rdbuf();
    file.close();

    /// 中身を保存
    std::string line;
    fileData_.push_back({ fileName, CSVData() });
    while (std::getline(fileStream, line))
    {
        std::stringstream lineStream(line);
        std::string word;
        // １行分確保
        fileData_.back().second.push_back({});
        while (std::getline(lineStream, word, ','))
        {
            // wordを追加
            fileData_.back().second.back().push_back({});
            fileData_.back().second.back().back() << word;
        }
    }

    return &fileData_.back().second;

}

CSVLine* CSVLoader::GetLine(std::string fileName, std::string ID)
{
    for (auto& file : fileData_)
    {
        if (file.first.compare(fileName) == 0)
        {
            for (auto& line : file.second)
            {
                if (line[0].str().compare(ID) == 0)
                {
                    return &line;
                }
            }
        }
    }
    return nullptr;
}

CSVLine* CSVLoader::GetNewLine(std::string fileName)
{
    for (auto& file : fileData_)
    {
        if (file.first.compare(fileName) == 0)
        {
            file.second.push_back({});
            return &file.second.back();
        }
    }
    return nullptr;
}

void CSVLoader::SaveFile()
{
    std::filesystem::path dir(directory_);
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    for (auto& file : fileData_)
    {

        std::string path = directory_ + file.first;
        std::ofstream ofile;
        ofile.open(path, std::ios::out | std::ios::trunc);
        if (!ofile.is_open())
        {
            MessageBoxA(nullptr, "ファイル書き込みに失敗", "CSVLoader.SaveFile", 0);
            return;
        }

        for (auto& line : file.second)
        {
            for (auto& word : line)
            {
                ofile << word.str() << ',';
            }
            ofile << '\n';
        }
        ofile.close();
    }

    return;
}

void CSVLoader::SaveLine(std::string fileName, CSVLine& line)
{
    std::filesystem::path dir(directory_);
    if (!std::filesystem::exists(dir))
    {
        std::filesystem::create_directory(dir);
    }

    std::string path = directory_ + fileName;
    std::ofstream ofile(path, std::ios::app);
    if (!ofile.is_open())
    {
        MessageBoxA(nullptr, "ファイル書き込みに失敗", "CSVLoader.SaveLine", 0);
        return;
    }
    for (auto& word : line)
    {
        ofile << word.str() << ",";
    }
    ofile << '\n';
    ofile.close();
    return;
}

CSVLoader::~CSVLoader()
{
    SaveFile();
}

void CSVLoader::DebugWindow()
{
#ifdef _DEBUG
    ImGui::BeginChild("FileListChild", ImVec2(100, 0), ImGuiChildFlags_Border);
    if (ImGui::BeginTable("## bg", 1, ImGuiTableFlags_RowBg))
    {
        for (auto& file : fileData_)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (ImGui::Selectable(file.first.c_str(), previewFile_ == file.first))
            {
                previewFile_ = file.first;
            }
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    std::string previewBuffer;
    for (auto& file : fileData_)
    {
        if (file.first.compare(previewFile_) != 0)
        {
            continue;
        }
        for (auto& line : file.second)
        {
            for (auto& word : line)
            {
                previewBuffer += word.str() + ", ";
            }
            previewBuffer += '\n';
        }
    }

    ImGui::BeginGroup();
    {
        ImGui::Text(previewFile_.c_str());
        if (previewFile_.size() != 0)
        {
            ImGui::TextDisabled((directory_ + previewFile_).c_str());
        }
        else ImGui::TextDisabled("");

        ImGui::Separator();

        ImGui::InputTextMultiline("##preview", previewBuffer.data(), previewBuffer.size(), ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
    }

    ImGui::EndGroup();
#endif
}
