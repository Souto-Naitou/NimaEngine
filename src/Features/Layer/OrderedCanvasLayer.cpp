#include "OrderedCanvasLayer.h"
#include <DebugTools/Logger/Logger.h>
#include <imgui.h>


bool IsCanvasEnabled(Canvas* canvas)
{
    bool isEnabled = canvas->IsEnabled();
    isEnabled &= !canvas->IsEnableManualDraw();
    return isEnabled;
}

void OrderedCanvasLayer::Initialize()
{
    pDebugEntry_ = std::make_unique<DebugEntry<OrderedCanvasLayer>>("Core", "OrderedCanvasLayer", this);
}

void OrderedCanvasLayer::ImGui()
{
    #ifdef _DEBUG

    if (ImGui::BeginTable(
        "PostEffectTable",
        4,
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Z", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Canvas Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed, 100.0f);

        // ヘッダー行の高さ
        const float headerRowHeight = ImGui::GetFrameHeight();

        // 手動でヘッダー行を描画（中央揃え対応）
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers, headerRowHeight);

        // ヘッダー用の垂直オフセット計算
        const float headerTextHeight = ImGui::GetTextLineHeight();
        const float headerOffsetY = (headerRowHeight - headerTextHeight) * 0.5f;

        // Z列ヘッダー
        ImGui::TableSetColumnIndex(0);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + headerOffsetY);
        ImGui::TextUnformatted("Z");

        // Type列ヘッダー
        ImGui::TableSetColumnIndex(1);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + headerOffsetY);
        ImGui::TextUnformatted("Type");

        // Canvas Name列ヘッダー
        ImGui::TableSetColumnIndex(2);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + headerOffsetY);
        ImGui::TextUnformatted("Canvas Name");

        // Enabled列ヘッダー（水平・垂直中央揃え）
        ImGui::TableSetColumnIndex(3);
        const char* enabledHeaderLabel = "Enabled";
        const float headerTextWidth = ImGui::CalcTextSize(enabledHeaderLabel).x;
        const float headerColumnWidth = ImGui::GetContentRegionAvail().x;
        const float headerOffsetX = (headerColumnWidth - headerTextWidth) * 0.5f;

        // 垂直・水平両方のオフセットを適用
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + headerOffsetY);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX);
        ImGui::TextUnformatted(enabledHeaderLabel);

        // データ行の高さ
        const float dataRowHeight = 28.0f;

        // データ行用の垂直オフセット計算
        const float dataTextHeight = ImGui::GetTextLineHeight();
        const float dataTextOffsetY = (dataRowHeight - dataTextHeight) * 0.5f;
        const float dataButtonHeight = ImGui::GetFrameHeight();
        const float dataButtonOffsetY = (dataRowHeight - dataButtonHeight) * 0.5f;

        // データ行の描画
        for (auto& [z, canvas] : canvases_)
        {
            ImGui::PushID(canvas);
            const bool isEnabled = canvas->IsEnabled();
            const bool isTopZ = (z >= kTopZBegin_);

            ImGui::TableNextRow(ImGuiTableRowFlags_None, dataRowHeight);

            // Z Order（垂直中央揃え）
            ImGui::TableSetColumnIndex(0);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + dataTextOffsetY);
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "0x%08X", z);

            // Type (Auto / Top)（垂直中央揃え）
            ImGui::TableSetColumnIndex(1);
            if (isTopZ)
            {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(40, 80, 120, 80));
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + dataTextOffsetY);
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Top");
            }
            else
            {
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + dataTextOffsetY);
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Auto");
            }

            // Canvas Name（垂直中央揃え）
            ImGui::TableSetColumnIndex(2);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + dataTextOffsetY);
            if (!isEnabled)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            ImGui::TextUnformatted(canvas->GetName().c_str());
            if (!isEnabled)
            {
                ImGui::PopStyleColor();
            }

            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Canvas:  %s", canvas->GetName().c_str());
                ImGui::Separator();
                ImGui::Text("Z: %u", z);
                ImGui::Text("Type: %s", isTopZ ? "Top" : "Auto");
                ImGui::Text("Enabled: %s", isEnabled ? "Yes" : "No");
                ImGui::EndTooltip();
            }

            // Enabled（水平・垂直中央揃え）
            ImGui::TableSetColumnIndex(3);

            // ボタンのサイズを定義
            const ImVec2 buttonSize(35.0f, 0.0f);

            // 現在の列の幅を取得
            const float columnWidth = ImGui::GetContentRegionAvail().x;

            // 水平中央揃えのためのオフセットを計算
            const float offsetX = (columnWidth - buttonSize.x) * 0.5f;

            // カーソル位置を垂直・水平中央に移動
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + dataButtonOffsetY);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

            bool enabled = isEnabled;
            if (enabled)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.5f, 0.15f, 1.0f));
                if (ImGui::Button("ON", buttonSize)) canvas->SetEnable(false);
                ImGui::PopStyleColor(3);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
                if (ImGui::Button("OFF", buttonSize)) canvas->SetEnable(true);
                ImGui::PopStyleColor(3);
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    #endif // _DEBUG
}

uint32_t OrderedCanvasLayer::AddCanvas(Canvas* canvas)
{
    uint32_t assignedZOrder = kAutoZBegin_;

    auto it = canvases_.upper_bound(kAutoZEnd_);

    if (it != canvases_.begin())
    {
        --it;
        if (it->first >= kAutoZEnd_)
        {
            LOG_FATAL("Cannot assign Z-order automatically. The auto-assignment range is full.");

            /// Zオーダーとキャンバス名をログに出力
            std::string strDump = "Current assigned Z-orders: { ";
            for (auto& pair : canvases_)
            {
                strDump += std::format("(Z:{}, Name:{}), ", pair.first, pair.second->GetName());
            }
            strDump += "}";
            LOG_FATAL(strDump);

            /// デバッグビルド時はアサート、リリースビルド時は強制終了
            #ifdef _DEBUG
            assert(false && "Cannot assign Z-order automatically. The auto-assignment range is full.");
            #else
            std::terminate();
            #endif // _DEBUG
        }

        assignedZOrder = it->first + 1;
    }

    canvases_.emplace(assignedZOrder, canvas);
    canvas->GetPostEffectExecutor().RegisterCommandListToDirectX12(assignedZOrder);

    return assignedZOrder;
}

uint32_t OrderedCanvasLayer::AddCanvasTop(Canvas* canvas)
{
    uint32_t assignedZOrder = kTopZBegin_;

    if (!canvases_.empty())
    {
        auto it = canvases_.rbegin();
        if (it->first >= kTopZBegin_)
        {
            assignedZOrder = it->first + 1;
        }
    }

    canvases_.emplace(assignedZOrder, canvas);
    canvas->GetPostEffectExecutor().RegisterCommandListToDirectX12(assignedZOrder);

    return assignedZOrder;
}

void OrderedCanvasLayer::RemoveCanvas(ZOrder zOrder)
{
    auto it = canvases_.find(zOrder);
    if (it != canvases_.end())
    {
        canvases_.erase(it);
    }
}

void OrderedCanvasLayer::RemoveCanvas(Canvas* canvas)
{
    for (auto it = canvases_.begin(); it != canvases_.end(); ++it)
    {
        if (it->second == canvas)
        {
            canvases_.erase(it);
            break;
        }
    }
}

void OrderedCanvasLayer::PreDraw()
{
    for (auto& canvas : canvases_)
    {
        canvas.second->GetPostEffectExecutor().PreDraw();
    }
}

void OrderedCanvasLayer::PostDraw()
{
    for (auto& canvas : canvases_)
    {
        canvas.second->GetPostEffectExecutor().PostDraw();
    }
}

void OrderedCanvasLayer::DrawObjects()
{
    // リストに格納された順番通りにCanvasのオブジェクトを描画する
    for (auto& canvas : canvases_)
    {
        if (IsCanvasEnabled(canvas.second))
        {
            canvas.second->DrawObjects();
        }
    }
}

void OrderedCanvasLayer::ApplyPostEffects()
{
    for (auto& canvas : canvases_)
    {
        if (IsCanvasEnabled(canvas.second))
        {
            canvas.second->ApplyPostEffects();
        }
    }
}

void OrderedCanvasLayer::DrawCanvases()
{
    for (auto& canvas : canvases_)
    {
        if (IsCanvasEnabled(canvas.second))
        {
            canvas.second->DrawCall(nullptr);
        }
    }
}
