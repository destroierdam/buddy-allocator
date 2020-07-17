#include "Allocator.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <exception>
#include <utility>
#include <string>
#include "Utility.h"

std::byte* Allocator::initBuffer(const std::size_t size) {
    logger.log(Logger::SeverityLevel::info, Logger::Action::none, "Allocating buffer");
    return static_cast<std::byte*>(malloc(size));
}

void Allocator::reserveDummy(std::size_t size) {
    if (size == 0) return;
    // Reserve
    logger.log(Logger::SeverityLevel::info, Logger::Action::none, 
               "Size not a power of 2, reserving " + Utility::stringFor(size) + " bytes");

    throw std::logic_error("Method not implemented");
    logger.log(Logger::SeverityLevel::info, Logger::Action::none, 
               "Available for allocation: " + Utility::stringFor(size));
    available = size;
}


void Allocator::initTree() {
    // Set every pointer in the array to nullptr
    memset(this->tree.data(), 0, this->tree.size() * sizeof(Block*));
    // Set root of tree
    this->tree[0] = reinterpret_cast<Block*>(this->buffer);
    this->tree[0]->next = nullptr;

    const std::size_t dummyMemorySize = workSize - allocatedSize;
    reserveDummy(dummyMemorySize);
}

std::size_t Allocator::blockLevelInTreeForSize(std::size_t size) {
    return log2(this->workSize) - log2(size);
}

std::size_t Allocator::sizeForLevel(std::size_t level) {
    return (this->workSize >> level);
}

Allocator::Block* Allocator::buddyOf(Block* block, std::size_t blockSize)
{
    // If we are looking for the buddy of the root block
    if (blockSize == this->workSize) {
        return block; // nullptr?
    }
    std::size_t indexOfBlock = (reinterpret_cast<std::byte*>(block) - this->buffer) / blockSize;
    std::size_t indexOfBuddy = indexOfBlock ^ 1;
    return reinterpret_cast<Block*>(this->buffer + indexOfBuddy* blockSize);
}

std::size_t Allocator::indexForBlock(Block* block, std::size_t blockSize)
{
    // index_in_level_of(p,n) == (p - _buffer_start) / size_of_level(n)
    std::size_t indexInLevel = (reinterpret_cast<std::byte*>(block) - this->buffer) / blockSize;
    return (1<<blockLevelInTreeForSize(blockSize)) + indexInLevel - 1;
}

void Allocator::collectGarbage() {
    std::size_t level = LEVELS - 1;

    while (level > 0) {
        while (this->tree[level]) {
            std::size_t size = sizeForLevel(level);
            Block* rogueBlock = buddyOf(reinterpret_cast<Block*>(this->tree[level]), size);
            
            /*std::array<char, 128> msg;
            "Block at address " + Utility::stringFor(rogueBlock) + " and size " + Utility::stringFor(size) + " was not deallocated";
            logger.log(Logger::SeverityLevel::warning, Logger::Action::leak, msg.data());*/
        }
        level--;
    }
    assert(this->tree[0] != nullptr);
}

void* Allocator::_allocate(std::size_t size) {
    std::size_t level = blockLevelInTreeForSize(size);

    // If a block with the correct size is available return it
    if (this->tree[level] != nullptr) {
        const std::size_t idx = indexForBlock(this->tree[level], sizeForLevel(level));
        this->mergelist.setBlock(idx, true);
        return std::exchange(this->tree[level], this->tree[level]->next);
    }

    // Go up the tree trying to find a block to split
    while (level != -1 && this->tree[level] == nullptr) {
        level--;
    }
    // If no memory is found
    if (level == -1) {
        throw std::bad_alloc();
    }

    do {
        // Get the block which must be split
        std::byte* blockToBeSplit = reinterpret_cast<std::byte*>(this->tree[level]);
        // Mark as split
        const std::size_t idx = indexForBlock(this->tree[level], sizeForLevel(level));
        this->mergelist.setBlock(idx, true);
        // Remove it from the list for it's size
        this->tree[level] = this->tree[level]->next;
        
        // Create its two children
        Block* left = reinterpret_cast<Block*>(blockToBeSplit);
        Block* right = reinterpret_cast<Block*>(blockToBeSplit + sizeForLevel(level + 1));
        // Add them to the list for the next level
        right->next = this->tree[level + 1];
        left->next = right;
        this->tree[level + 1] = left;
        level++;
    } while (level < blockLevelInTreeForSize(size));

    const std::size_t idx = indexForBlock(this->tree[level], sizeForLevel(level));
    this->mergelist.setBlock(idx, true);

    return std::exchange(this->tree[level], this->tree[level]->next);
}

void Allocator::_deallocate(Block* block, std::size_t size) {
    Block* buddy = buddyOf(block, size);
    std::size_t blockIdx = indexForBlock(block, size);
    std::size_t buddyIdx = indexForBlock(buddy, size);
    std::size_t level = blockLevelInTreeForSize(size);

    // Mark block as free
    this->mergelist.free(blockIdx);

    // If we have deallocated all memory(reached the root of the tree)
    if (size == this->workSize) {
        block->next = nullptr;
        this->tree[0] = block;
        return;
    }

    // If it's buddy is not available then we do not merge
    if (this->mergelist.canBeFreed(buddyIdx)) { // .buddyIsAvailable(buddyIdx)
        // Add the block in the free list
        block->next = this->tree[level];
        this->tree[level] = block;
        return;
    }
    // Buddy is available
    // Merge all free blocks and continue recursively up the tree

    // Remove buddy from the free list
    Block** it = &(this->tree[level]);
    // TODO: Refactor this
    while (true) {
        if (*it == buddy) {
            *it = (*it)->next;
            break;
        }
        it = &((*it)->next);
    }
    Block* mergedBlock = std::min(block, buddy);
    size <<= 1;
    level--;
    _deallocate(mergedBlock, size);
}

Allocator::Allocator(const std::size_t size, std::ostream& logStream) : buffer(initBuffer(size)),
workSize(Utility::closestBiggerPowerOf2(size)),
allocatedSize(size),
MIN_ALLOC_BLOCK_SIZE(16),
logger(logStream)
{
    LEVELS = log2(workSize) - log2(MIN_ALLOC_BLOCK_SIZE) + 1;

    logger.log(Logger::SeverityLevel::info, Logger::Action::none, 
               "Allocator constructed with size " + Utility::stringFor(size));

    logger.log(Logger::SeverityLevel::info, Logger::Action::none, 
               "Working size: " + Utility::stringFor(workSize));

    memset(this->buffer, 'F', this->allocatedSize); // 'F'ree
    initTree();
}

Allocator::~Allocator() {
    if (this->tree[0] == nullptr) {
        logger.log(Logger::SeverityLevel::warning, Logger::Action::leak, "Memory leak");
        // collectGarbage();
    }
    assert(used == available);
    free(this->buffer);
}

void* Allocator::allocate(std::size_t size) {
    logger.log(Logger::SeverityLevel::info, Logger::Action::allocation, 
               "Request for allocation of " + Utility::stringFor(size) + " bytes");

    size = Utility::closestBiggerPowerOf2(size);
    size = std::max(size, this->MIN_ALLOC_BLOCK_SIZE);

    logger.log(Logger::SeverityLevel::info, Logger::Action::allocation, 
               "Will be allocated " + Utility::stringFor(size) + " bytes");

    void* const ans = _allocate(size);

    logger.log(Logger::SeverityLevel::info, Logger::Action::allocation,
        "Block with address " + Utility::stringFor((long)(ans)) + " is allocated");

    memset(ans, 'A', size); // 'A'llocated
    return ans;
}

void* Allocator::allocate(std::size_t size, std::nothrow_t) noexcept
{
    try {
        return this->allocate(size);
    } catch (std::bad_alloc& ex) {
        logger.log(Logger::SeverityLevel::error, Logger::Action::exception, ex.what());
        return nullptr;
    } catch (...) {
        logger.log(Logger::SeverityLevel::error, Logger::Action::exception, 
                   "Unknown exception during allocation");
        return nullptr;
    }
}

void Allocator::deallocate(void* address, std::size_t size) noexcept {
    logger.log(Logger::SeverityLevel::info, Logger::Action::deallocation,
        "Request for deallocation of block " + Utility::stringFor((long)(address)) + " with size " + 
         Utility::stringFor(size) + " bytes");

    size = Utility::closestBiggerPowerOf2(size);
    size = std::max(size, this->MIN_ALLOC_BLOCK_SIZE);
    memset(address, 'D', size); // 'D'eallocated
    _deallocate(reinterpret_cast<Block*>(address), size);

    logger.log(Logger::SeverityLevel::info, Logger::Action::deallocation,
        "Deallocation of block " + Utility::stringFor((long)(address)) + " with size " +
        Utility::stringFor(size) + " bytes completed");
}
