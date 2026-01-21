#include "LineBatch.h"



void LineBatch::Initialize()
{
    pLine_ = std::make_unique<Line>(kMaxLines_);
}

void LineBatch::Update()
{
    pLine_->Update();
}

void LineBatch::Draw1F()
{
    pLine_->Draw1F();
}

AllocatedLineVertices LineBatch::Allocate(size_t numReserveLine)
{
    const size_t kNumReserveVertex = numReserveLine * 2;
    auto freeOffset = this->FindFreeOffset(kNumReserveVertex);
    this->ReserveFreeBlock(freeOffset, kNumReserveVertex);

    Vector3* pVertexData = &((*pLine_)[freeOffset]);
    return AllocatedLineVertices(this, freeOffset, kNumReserveVertex, pVertexData);
}

void LineBatch::Deallocate(uint32_t offset, size_t size)
{
    FreeBlock newBlock;
    newBlock.offset = offset;
    newBlock.size = size;
    freeBlocks_.push_back(newBlock);

    this->MergeFreeBlocks();
}

uint32_t LineBatch::FindFreeOffset(size_t size)
{
    for (const auto& block : freeBlocks_)
    {
        if (block.size >= size)
        {
            return block.offset;
        }
    }

    assert(false && "LineBatch: No free block available for allocation.");
    return 0;
}

void LineBatch::ReserveFreeBlock(uint32_t offset, size_t size)
{
    for (auto& block : freeBlocks_)
    {
        if (block.offset == offset)
        {
            block.offset += static_cast<uint32_t>(size);
            block.size -= size;
        }
    }
}

void LineBatch::MergeFreeBlocks()
{
    std::sort(
        freeBlocks_.begin(), freeBlocks_.end(),
        [](const FreeBlock& a, const FreeBlock& b) {
        return a.offset < b.offset;
    });

    std::vector<FreeBlock> mergedBlocks;
    mergedBlocks.reserve(freeBlocks_.size());

    for (const auto& block : freeBlocks_)
    {
        if (mergedBlocks.empty())
        {
            mergedBlocks.push_back(block);
            continue;
        }
        else
        {
            FreeBlock& lastBlock = mergedBlocks.back();
            if (lastBlock.offset + lastBlock.size == block.offset)
            {
                lastBlock.size += block.size;
            }
            else
            {
                mergedBlocks.push_back(block);
            }
        }
    }
    freeBlocks_.swap(mergedBlocks);
}