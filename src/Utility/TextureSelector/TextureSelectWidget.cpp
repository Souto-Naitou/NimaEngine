#include "TextureSelectWidget.h"

#include <Core/ConfigManager/ConfigManager.h>

#include <imgui.h>
#include <algorithm>
#include <cstdint>
#include <imgui_internal.h>
#include <Color.h>
#include <ImGui/imgui_stdlib.h>

namespace fs = std::filesystem;


void TextureSelectWidget::Initialize()
{
    auto tm = TextureManager::GetInstance();
    tm->LoadTexture("folder.png");
    tm->LoadTexture("upward.png");
}

bool TextureSelectWidget::DrawSelector(const TextureManager::TextureData*& outTextureData, float maxWidth)
{
    #ifdef _DEBUG

    bool isSelected = false;

    TextureManager* pTextureManager = TextureManager::GetInstance();
    auto& style = ImGui::GetStyle();

    ImGui::BeginChild("", ImVec2(0, maxWidth), true);

    // デフォルトのテクスチャパスを取得
    if (directoryCacheMap_.empty())
    {
        auto& texPathList = ConfigManager::GetInstance()->GetConfigData().texture_paths;
        if (!texPathList.empty())
        {
            directoryCacheMap_[texPathList.back()] = false;
            currentDirectory_ = texPathList.back();
        }
        else
        {
            directoryCacheMap_[fs::current_path()] = false;
            currentDirectory_ = fs::current_path();
        }
        currentDirectory_ = fs::absolute(currentDirectory_);
    }

    /// 現在表示しているディレクトリを表示する
    {
        // std::filesystem::path::string()は一時オブジェクトを返すため、std::stringに変換してから渡す必要がある
        std::string currentDirStr = currentDirectory_.string();
        ImGui::InputText("##CurrentDirectory", &currentDirStr, ImGuiInputTextFlags_ReadOnly);
    }

    /// 親ディレクトリに移動するボタンを表示する
    {
        // InputTextと同じ高さ
        const float target = ImGui::GetFrameHeight();

        // 画像部分の高さ
        const float sizeY = target - ImGui::GetStyle().FramePadding.y * 2.0f;

        auto& resource = pTextureManager->GetTextureResource("upward.png");
        ImTextureID texId = static_cast<ImTextureID>(resource.GetSRVHandleGPU().ptr);
        ImVec2 buttonSize = { sizeY, sizeY };
        ImGui::SameLine();
        if (ImGui::ImageButton("##upward", texId, buttonSize))
        {
            // 親ディレクトリに移動する
            MoveToParentDirectory();
        }
        pImGuiManager_->AddImageResource(&resource);
    }

    /// 現在のディレクトリにあるテクスチャを予め読み込んでおく
    bool& isPreloaded = directoryCacheMap_[currentDirectory_];
    if (isPreloaded == false)
    {
        PreloadDirectory(currentDirectory_);
        isPreloaded = true;
    }

    // 折り返し判定に使う「表示領域の右端X」（スクロールバー・パディング込み）
    const float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

    for (auto& entry : fs::directory_iterator(currentDirectory_))
    {
        // ディレクトリかどうか
        bool isDirectory = entry.is_directory();

        // テクスチャデータ
        TextureManager::TextureData* pTextureData = nullptr;


        /// ディレクトリの場合
        if (isDirectory)
        {
            // フォルダアイコンを表示する
            pTextureData = &pTextureManager->GetTextureData("folder.png");
        }
        else
        {
            std::string ext = entry.path().extension().string();
            if (ext != ".png" && ext != ".dds") continue;
            pTextureData = &pTextureManager->GetTextureData(entry.path().string());
        }

        if (pTextureData->metadata.IsCubemap())
        {
            continue;
        }

        // ファイル / フォルダーのパス
        std::string path = entry.path().string();
        ImTextureID textureID = static_cast<ImTextureID>(pTextureData->textureResource.GetSRVHandleGPU().ptr);

        ImGui::PushID(path.c_str());

        uint32_t width = static_cast<uint32_t>(pTextureData->metadata.width);
        uint32_t height = static_cast<uint32_t>(pTextureData->metadata.height);
        uint32_t mainAxis = static_cast<uint32_t>(std::max(pTextureData->metadata.width, pTextureData->metadata.height));

        const ImVec2 buttonSize = { buttonSizeMax_.x * width / mainAxis, buttonSizeMax_.y * height / mainAxis };

        if (isDirectory)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.69f, 0.93f, 1.0f, 0.0f));
            if (ImGui::ImageButton("##icon", textureID, buttonSize))
            {
                // ディレクトリの場合はそのディレクトリに移動する
                currentDirectory_ = entry.path();
                currentDirectory_ = fs::absolute(currentDirectory_);
            }
            ImGui::PopStyleColor();
        }
        else
        {
            if (ImGui::ImageButton("##icon", textureID, buttonSize))
            {
                // ファイルの場合は選択されたテクスチャとして返す
                isSelected = true;
                outTextureData = pTextureData;
            }
        }
        

        if (ImGui::IsItemHovered())
        {
            if (ImGui::BeginTooltipEx(ImGuiTooltipFlags_OverridePrevious, ImGuiWindowFlags_None))
            { 
                std::string text = "Width: " + std::to_string(width) + "\nHeight: " + std::to_string(height);
                Vector4 color = RGBA(0x65bad8ff).to_Vector4();

                ImGui::TextColored(ImVec4(color.x, color.y, color.z, color.w), "[path]");
                ImGui::Indent();
                ImGui::Text(path.c_str());
                ImGui::Unindent();

                ImGui::TextColored(ImVec4(color.x, color.y, color.z, color.w), "[metadata]");
                ImGui::Indent();
                ImGui::Text(text.c_str());
                ImGui::Unindent();

                ImGui::EndTooltip();
            }
        }

        // 折り返しロジック
        const float lastX2 = ImGui::GetItemRectMax().x;

        // 次ボタンの右端 = 今の右端 + 項目間隔 + (画像幅 + 枠パディング左右)
        const float nextX2 = lastX2 + style.ItemSpacing.x + (buttonSize.x + style.FramePadding.x * 2.0f);
        if (nextX2 < windowVisibleX2)
        {
            ImGui::SameLine();
        }

        pImGuiManager_->AddImageResource(&pTextureData->textureResource);

        ImGui::PopID();
    };

    // ズームスライダーの表示
    ImGui::NewLine();
    ImGui::SliderFloat2("##ButtonSize", &buttonSizeMax_.x, 16.0f, 128.0f, "%.0f");

    ImGui::EndChild();
    return isSelected;
    #else
    outTextureData;
    maxWidth;
    #endif // _DEBUG
}

void TextureSelectWidget::PreloadDirectory(const std::filesystem::path& directoryPath)
{
    auto tm = TextureManager::GetInstance();

    /// ディレクトリ内のテクスチャを読み込む
    for (auto& entry : fs::directory_iterator(directoryPath))
    {
        if (entry.is_regular_file())
        {
            std::string ext = entry.path().extension().string();

            // 拡張子を小文字に変換して比較する
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".png" || ext == ".dds")
            {
                auto loaded = tm->LoadTexture(entry.path().string());
            }
        }
    }
}

void TextureSelectWidget::MoveToParentDirectory()
{
    currentDirectory_ = currentDirectory_.parent_path();
    currentDirectory_ = fs::absolute(currentDirectory_);
}
