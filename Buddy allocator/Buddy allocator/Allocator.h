#pragma once
#include <cstddef>
#include <array>
class Allocator {
private:
    struct Block {
        Block* next;
    };

    /// Returns the index of the block in the tree which corresponds to the block size
    std::size_t blockForSize(std::size_t size);

    /// Given the level in the tree calculates the size of the blocks on that level
    std::size_t sizeForLevel(std::size_t level);

    const std::size_t MIN_ALLOC_BLOCK_SIZE;
    /// The depth of the tree
    std::size_t LEVELS;
    std::array<Block*, 32> tree;

    std::byte* const buffer;
    /// The theoretical size of the buffer the algorithm will work with
    const std::size_t workSize;
    
    /// Number of bytes actually allocated from the OS
    const std::size_t allocatedSize;
    
    /// Initial allocation of the buffer
    std::byte* initBuffer(const std::size_t size);
    
    /// If the allocated memory is not a power of 2 this function will mark the difference as allocated
    void initTree();

    void reserveDummy(std::size_t size);
    void* _allocate(std::size_t size);
public:
    Allocator(const std::size_t size);
    ~Allocator();

    void* allocate(std::size_t size);
    void deallocate(void* address, const std::size_t size);
};