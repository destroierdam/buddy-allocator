#include "Allocator.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "Utility.h"

std::byte* Allocator::initBuffer(const std::size_t size) {
    return static_cast<std::byte*>(malloc(size));
}

Allocator::Allocator(const std::size_t size) : buffer(initBuffer(size)),
workSize(Utility::closestBiggerPowerOf2(size)),
allocatedSize(size),
MIN_ALLOC_BLOCK_SIZE(64) {
    memset(this->buffer, 0, this->allocatedSize);
}

Allocator::~Allocator() {
    free(this->buffer);
}

void* Allocator::allocate(std::size_t size) {
    return nullptr;
}

void Allocator::deallocate(void* address, const std::size_t size) {
    memset(address, -1, size);
}