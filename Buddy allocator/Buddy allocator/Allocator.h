#pragma once
#include <cstddef>
class Allocator {
private:
    const std::size_t MIN_ALLOC_BLOCK_SIZE;

    std::byte* const buffer;
    const std::size_t workSize;
    const std::size_t allocatedSize;
    std::byte* initBuffer(const std::size_t size);
public:
    Allocator(const std::size_t size);
    ~Allocator();

    void* allocate(std::size_t size);
    void deallocate(void* address, const std::size_t size);
};

