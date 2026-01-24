#pragma once
#include <vector>
#include <Features/Collision/Collider/Collider.h>
#include <Features/Collision/Manager/CollisionManager.h>
#include <set>
#include <drawable/line/Line.h>
#include <memory>

// コライダーのセル分割によるフィルタリング
// 衝突判定マネージャ登録ヘルパークラス
class CellBasedFiltering
{
public:
    CellBasedFiltering() = default;
    ~CellBasedFiltering() = default;

    void Initialize(int cellSize, int maxWorldX, int maxWorldZ);
    void Draw2dDebug();
    void DrawImGui();

    void AssignToGrid(Collider* collider);
    void RegisterPotentials(Collider* pCollider);
    void RegisterAll(CollisionManager* pManager);
    void UnregisterAll(CollisionManager* pManager);
    void ReassignToGridAll(int cellSize);

private:
    uint64_t ToCellIndex(const Vector3& position) const;

    void Draw2dPotentials();
    void Draw2dCurrentCells();
    void Draw2dAllCells();

    int cellSize_       = 0;
    int worldWidth_     = 0;
    int worldHeight_    = 0;
    int numCellsX_      = 0;
    int numCellsZ_      = 0;

    std::vector<std::vector<Collider*>> grid_ = {};
    std::set<Collider*> potentialColliders_ = {};
    std::set<uint64_t> activeCellsIndices_ = {};
    std::unique_ptr<Line> cellGridLines_ = {};
    bool isModifyMode_ = false;
    bool isDrawPotentials_ = false;
    bool isDrawCurrentCells_ = false;
    bool isDrawAllCells_ = false;
    float cellGridY_ = 0.0f; // 2D描画用のY位置
};