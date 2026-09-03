#include "TextureSelectWidget.h"

#include <imgui.h>
#include <algorithm>
#include <cstdint>

bool TextureSelectWidget::DrawSelector(const TextureManager::TextureData*& outTextureData, float maxWidth)
{
    #ifdef _DEBUG

    bool isSelected = false;

    TextureManager* pTextureManager = TextureManager::GetInstance();
    TextureManager::TextureDataMap& textureDataMap = pTextureManager->GetTextureDataMap();
    auto& style = ImGui::GetStyle();

    ImGui::BeginChild("", ImVec2(0, maxWidth), true);

    ImGui::SliderFloat2("##ButtonSize", &buttonSizeMax_.x, 16.0f, 128.0f, "%.0f");

    // 折り返し判定に使う「表示領域の右端X」（スクロールバー・パディング込み）
    const float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

    for (auto& [filePath, textureData] : textureDataMap)
    {
        if (textureData.metadata.IsCubemap())
        {
            continue;
        }

        std::string path = filePath.string();
        ImTextureID textureID = static_cast<ImTextureID>(textureData.textureResource.GetSRVHandleGPU().ptr);

        ImGui::PushID(path.c_str());

        uint32_t width = static_cast<uint32_t>(textureData.metadata.width);
        uint32_t height = static_cast<uint32_t>(textureData.metadata.height);
        uint32_t mainAxis = static_cast<uint32_t>(std::max(textureData.metadata.width, textureData.metadata.height));

        const ImVec2 buttonSize = { buttonSizeMax_.x * width / mainAxis, buttonSizeMax_.y * height / mainAxis };

        if (ImGui::ImageButton("", textureID, buttonSize))
        {
            isSelected = true;
            outTextureData = &textureData;
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", filePath.string().c_str());

        // 折り返しロジック
        const float lastX2 = ImGui::GetItemRectMax().x;

        // 次ボタンの右端 = 今の右端 + 項目間隔 + (画像幅 + 枠パディング左右)
        const float nextX2 = lastX2 + style.ItemSpacing.x + (buttonSize.x + style.FramePadding.x * 2.0f);
        if (nextX2 < windowVisibleX2)
        {
            ImGui::SameLine();
        }

        pImGuiManager_->AddImageResource(&textureData.textureResource);

        ImGui::PopID();
    };

    ImGui::EndChild();
    return isSelected;
    #endif // _DEBUG
}
