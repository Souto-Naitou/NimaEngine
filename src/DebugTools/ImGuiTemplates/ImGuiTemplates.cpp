#include "ImGuiTemplates.h"

#ifdef _DEBUG

#include <string>
#include <string.h>
#include <filesystem>
#include <WinTools/WinTools.h>

void ImGuiTemplate::VariableTable(const std::string& stringID, std::function<void()> function)
{
    std::string id = stringID + "_VARTABLE";
    ImGui::PushID(id.c_str());
    bool result = ImGui::BeginTable(stringID.c_str(), 2, ImGuiTableFlags_Hideable | ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders);
    if (result)
    {
        ImGui::TableSetupColumn("変数名");
        ImGui::TableSetupColumn("値");
        ImGui::TableHeadersRow();

        function();

        ImGui::EndTable();
    }

    ImGui::PopID();
    return;
}

void ImGuiTemplate::FileWidget::ImGui(const std::string& id)
{
    ImGui::PushID(id.c_str());
    char path[512] = "";
    strncpy_s(path, sizeof(path), filePath_.c_str(), _TRUNCATE);

    if (ImGui::InputText("ファイルパス", path, sizeof(path)))
    {
        isFileExist_ = std::filesystem::exists(path);
    }

    if (ImGui::Button("保存"))
    {
        // コールバック呼び出し
        if (pOnSave_)
        {
            pOnSave_(path);
        }
    }
    
    ImGui::SameLine();

    if (ImGui::Button("読み込み"))
    {
        isFileExist_ = std::filesystem::exists(path);
        if (isFileExist_)
        {
            // コールバック呼び出し
            if (pOnLoad_)
            {
                pOnLoad_(path);
            }
        }
        else
        {
            isFileExist_ = false;
        }
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("ファイル選択..."))
    {
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::string temp = WinTools::GetInstance()->OpenFileDialog(currentPath.string());  
        if (!temp.empty())
        {
            strncpy_s(path, sizeof(path), temp.c_str(), _TRUNCATE);
            isFileExist_ = true;
        }
        else isFileExist_ = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("フォルダ選択..."))
    {
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::string temp = WinTools::GetInstance()->OpenFolderDialog(currentPath.string());
        if (!temp.empty())
        {
            strncpy_s(path, sizeof(path), temp.c_str(), _TRUNCATE);
            isFileExist_ = true;
        }
        else isFileExist_ = false;
    }

    if (!isFileExist_)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), messageMissing_.c_str());
    }

    ImGui::PopID();

    filePath_ = path;
}

#endif // DEBUG