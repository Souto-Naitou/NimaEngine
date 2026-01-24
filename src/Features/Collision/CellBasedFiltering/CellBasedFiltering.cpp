#include "CellBasedFiltering.h"
#include <array>
#include <set>
#include <drawable/line/Line.h>
#include <Color.h>
#include <imgui.h>


void CellBasedFiltering::Initialize(int cellSize, int worldWidth, int worldHeight)
{
    cellSize_ = cellSize;
    worldWidth_ = worldWidth;
    worldHeight_ = worldHeight;
    numCellsX_ = (worldWidth_ + cellSize_ - 1) / cellSize_;
    numCellsZ_ = (worldHeight_ + cellSize_ - 1) / cellSize_;
    grid_.resize(static_cast<size_t>(numCellsX_ * numCellsZ_));
}

void CellBasedFiltering::UnregisterAll(CollisionManager* pManager)
{
    for (const auto& collider : potentialColliders_)
    {
        pManager->DeleteCollider(collider);
    }
    potentialColliders_.clear();
}

void CellBasedFiltering::ReassignToGridAll(int cellSize)
{
    cellSize_ = cellSize;
    numCellsX_ = (worldWidth_ + cellSize_ - 1) / cellSize_;
    numCellsZ_ = (worldHeight_ + cellSize_ - 1) / cellSize_;
    std::vector<Collider*> assignedColliders{};
    for (auto& cell : grid_)
    {
        for (auto& collider : cell)
        {
            assignedColliders.push_back(collider);
        }
    }

    grid_.clear();
    grid_.resize(static_cast<size_t>(numCellsX_ * numCellsZ_));

    for (auto& collider : assignedColliders)
    {
        AssignToGrid(collider);
    }
}

void CellBasedFiltering::Draw2dDebug()
{
    if (isDrawPotentials_)
    {
        Draw2dPotentials();
    }
    if (isDrawCurrentCells_)
    {
        Draw2dCurrentCells();
    }
    if (isDrawAllCells_)
    {
        Draw2dAllCells();
    }
}

void CellBasedFiltering::DrawImGui()
{
    #ifdef _DEBUG
    if (ImGui::TreeNode("Display lines"))
    {
        ImGui::Indent(15.0f);

        ImGui::Checkbox("All Cells", &isDrawAllCells_);
        ImGui::Checkbox("Current Cells", &isDrawCurrentCells_);
        ImGui::Checkbox("Potentials", &isDrawPotentials_);

        if (isDrawAllCells_) ImGui::DragFloat("Cell Grid Y", &cellGridY_, 0.01f);

        ImGui::Unindent(15.0f);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Cells"))
    {
        ImGui::Indent(15.0f);

        ImGui::Checkbox("Enable modify", &isModifyMode_);

        if (!isModifyMode_)
        {
            ImGui::Text("Cell Size: %d", cellSize_);
        }
        else
        {
            ImGui::SliderInt("Cell Size", &cellSize_, 2, 40);
            if (ImGui::Button("Reassign All"))
            {
                ReassignToGridAll(cellSize_);
                isModifyMode_ = false;
            }
        }

        ImGui::Spacing();

        // 各種情報の表示
        ImGui::Text("World Size: %d x %d", worldWidth_, worldHeight_);
        ImGui::Text("Num Cells: %d x %d", numCellsX_, numCellsZ_);
        ImGui::Text("Total Cells: %zu", grid_.size());
        ImGui::Text("Potential Colliders: %zu", potentialColliders_.size());

        // 現在のアクティブセルの表示
        if (!activeCellsIndices_.empty())
        {
            std::string activeCellsStr = "Active Cells: { ";
            for (auto& activeCell : activeCellsIndices_)
            {
                activeCellsStr += std::to_string(activeCell) + " ";
            }
            activeCellsStr += "}";
            ImGui::Text("%s", activeCellsStr.c_str());
        }

        ImGui::Unindent(15.0f);
        ImGui::TreePop();
    }
    #endif
}

void CellBasedFiltering::AssignToGrid(Collider* collider)
{
    auto aabb = collider->GetShapeData<AABB>();
    Vector3 min = aabb->GetMin();
    Vector3 max = aabb->GetMax();
    Vector3 corners[4] = {
        {min.x, 0.0f, min.z},
        {max.x, 0.0f, min.z},
        {min.x, 0.0f, max.z},
        {max.x, 0.0f, max.z}
    };

    std::set<uint64_t> uniqueCells{};
    for (const auto& corner : corners)
    {
        uint64_t cellIndex = ToCellIndex(corner);
        uniqueCells.insert(cellIndex);
    }

    for (const auto& cellIndex : uniqueCells)
    {
        if (cellIndex < grid_.size())
        {
            grid_[cellIndex].push_back(collider);
        }
    }
}

void CellBasedFiltering::RegisterPotentials(Collider* pCollider)
{
    const AABB* aabb = pCollider->GetShapeData<AABB>();

    Vector3 min = aabb->GetMin();
    Vector3 max = aabb->GetMax();

    Vector3 corners[4] = {
        {min.x, 0.0f, min.z},
        {max.x, 0.0f, min.z},
        {min.x, 0.0f, max.z},
        {max.x, 0.0f, max.z}
    };

    activeCellsIndices_.clear();
    for (const auto& corner : corners)
    {
        uint64_t cellIndex = ToCellIndex(corner);
        if (cellIndex < grid_.size())
            activeCellsIndices_.insert(cellIndex);
    }

    for (const auto& cellIndex : activeCellsIndices_)
    {
        if (cellIndex < grid_.size())
        {
            for (const auto& collider : grid_[cellIndex])
            {
                potentialColliders_.insert(collider);
            }
        }
    }
}

void CellBasedFiltering::RegisterAll(CollisionManager* pManager)
{
    for (const auto& collider : potentialColliders_)
    {
        pManager->RegisterCollider(collider);
    }
}

uint64_t CellBasedFiltering::ToCellIndex(const Vector3& position) const
{
    // ワールド座標からセルのインデックスを計算
    uint64_t ix = static_cast<uint64_t>(position.x) / cellSize_;
    uint64_t iz = static_cast<uint64_t>(position.z) / cellSize_;
    return numCellsX_ * iz + ix;
}

void CellBasedFiltering::Draw2dPotentials()
{
    for (auto& collider : potentialColliders_)
    {
        auto aabb = collider->GetShapeData<AABB>();
        aabb->SetColor(0xff00ffff);
        aabb->Draw1F();
    }
}

void CellBasedFiltering::Draw2dCurrentCells()
{
    // 現在のセルを強調表示
    for (const auto& cellIndex : activeCellsIndices_)
    {
        if (cellIndex < grid_.size())
        {
            int ix = static_cast<int>(cellIndex % numCellsX_);
            int iz = static_cast<int>(cellIndex / numCellsX_);
            AABB cellAABB = 
            {
                Vector3{static_cast<float>(ix * cellSize_), -100.0f, static_cast<float>(iz * cellSize_)},
                Vector3{static_cast<float>((ix + 1) * cellSize_), 100.0f, static_cast<float>((iz + 1) * cellSize_)}
            };

            cellAABB.SetColor(0x0000ffff);
            cellAABB.Update();
            cellAABB.Draw1F();
        }
    }
}

void CellBasedFiltering::Draw2dAllCells()
{
    if (!cellGridLines_) cellGridLines_ = std::make_unique<Line>(numCellsX_ + 1 + numCellsZ_ + 1);

    if (cellGridLines_->GetLineCount() != static_cast<size_t>(numCellsX_ + 1 + numCellsZ_ + 1))
    {
        cellGridLines_->ResizeLine(static_cast<size_t>((numCellsX_ + 1 + numCellsZ_ + 1)));
    }

    size_t lineIdx = 0;
    for (int ix = 0; ix < numCellsX_ + 1; ++ix)
    {
        (*cellGridLines_)[lineIdx++] = Vector3{ static_cast<float>(ix * cellSize_), cellGridY_, 0.0f };
        (*cellGridLines_)[lineIdx++] = Vector3{ static_cast<float>(ix * cellSize_), cellGridY_, static_cast<float>(numCellsZ_ * cellSize_) };
    }
    for (int iz = 0; iz < numCellsZ_ + 1; ++iz)
    {
        (*cellGridLines_)[lineIdx++] = Vector3{ 0.0f, cellGridY_, static_cast<float>(iz * cellSize_) };
        (*cellGridLines_)[lineIdx++] = Vector3{ static_cast<float>(numCellsX_ * cellSize_), cellGridY_, static_cast<float>(iz * cellSize_) };
    }
    cellGridLines_->SetColor(RGBA(0xffff00ff).to_Vector4());
    cellGridLines_->Update();
    cellGridLines_->Draw1F();
}
