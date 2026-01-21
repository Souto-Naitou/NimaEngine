#pragma once

#include <span>
#include <Vector3.h>

class LineBatch;

class AllocatedLineVertices
{
public:
    AllocatedLineVertices(
        LineBatch* pLineBatch, uint32_t offset, size_t size, Vector3* ptr) :
        pOwner_(pLineBatch), vertices_(ptr, size), offset_(offset), size_(size) {}

    ~AllocatedLineVertices();

    std::span<Vector3>& Get() { return vertices_; }
    const std::span<Vector3>& Get() const { return vertices_; }

private:
    LineBatch* pOwner_;
    std::span<Vector3> vertices_;
    uint32_t offset_ = 0;
    size_t size_ = 0;
};