#pragma once
#include <memory>
#include <drawable/line/Line.h>
#include "AllocatedLineVertices.h"

/// <summary>
/// ラインまとめ描画クラス
/// </summary>
class LineBatch
{
public:
    struct FreeBlock
    {
        uint32_t offset = 0;
        size_t size = 0;
    };

    LineBatch(const LineBatch&) = delete;
    LineBatch& operator=(const LineBatch&) = delete;
    LineBatch(LineBatch&&) = delete;
    LineBatch& operator=(LineBatch&&) = delete;

    static LineBatch* GetInstance()
    {
        static LineBatch instance;
        return &instance;
    }

    void Initialize();
    void Update();
    void Draw1F();

    AllocatedLineVertices Allocate(size_t numReserveLine);
    void Deallocate(uint32_t offset, size_t size);

private:
    LineBatch() = default;
    ~LineBatch() = default;
    
    uint32_t FindFreeOffset(size_t size);
    void ReserveFreeBlock(uint32_t offset, size_t size);
    void MergeFreeBlocks();

    static constexpr size_t kMaxLines_ = 1024;
    std::unique_ptr<Line> pLine_ = nullptr;
    std::vector<FreeBlock> freeBlocks_;
};