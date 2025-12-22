#include "ImGuiTemplates.h"

#ifdef _DEBUG

#include <string>

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

#endif // DEBUG