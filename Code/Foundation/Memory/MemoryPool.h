#pragma once
#include <cstddef>

class MemoryPool {
public:
    MemoryPool(size_t size) : totalBytes_(size), usedBytes_(0) {}
    void Resize(size_t size) { totalBytes_ = size; }
    size_t GetUsedBytes() const { return usedBytes_; }
    size_t GetTotalBytes() const { return totalBytes_; }
    void Reset() { usedBytes_ = 0; }
private:
    size_t totalBytes_;
    size_t usedBytes_;
}; 