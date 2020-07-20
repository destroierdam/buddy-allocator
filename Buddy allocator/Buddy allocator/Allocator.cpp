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
#include <cmath>
#include "Utility.hpp"

std::byte* Allocator::initBuffer(const std::size_t size) {
    logger.log(Logger::SeverityLevel::info, Logger::Action::none, "Allocating buffer");
    return static_cast<std::byte*>(malloc(size));
}

void Allocator::reserveDummy(std::size_t size) {
    if (size == 0) return;
    // Reserve
    logger.log(Logger::SeverityLevel::info, Logger::Action::none, 
               "Size not a power of 2, reserving " + Utility::stringFor(size) + " bytes");

    std::size_t leafsNeeded = static_cast<std::size_t>(ceil((double)size / MIN_ALLOC_BLOCK_SIZE));

    // Go to all the leafs, mark them as allocated and mark all of their parents as split
    std::size_t idxLeafs = indexForBlock(reinterpret_cast<Block*>(this->workBuffer), MIN_ALLOC_BLOCK_SIZE);

    for (std::size_t i = 0; i < leafsNeeded; i++) {
        this->freeTable.take(idxLeafs);
        std::size_t parent = (idxLeafs - 1) / 2;

        // Mark parents as split
        for (std::size_t level = LEVELS - 2; level != -1 && this->splitTable[parent] == 0; level--) {
            this->splitTable.set(parent, true);
            this->freeTable.take(parent);
            parent = (parent - 1) / 2;
        }
        idxLeafs++;
    }

    // Fill linked lists
    std::size_t nodeIdx = 0;
    std::size_t level = 0;
    while (level < LEVELS-1) {
        std::size_t leftIdx = 2 * nodeIdx + 1;
        std::size_t rightIdx = 2 * nodeIdx + 2;
        // [[likely]]
        if (this->splitTable[nodeIdx]) {
            if (this->splitTable[rightIdx]) {
                nodeIdx = rightIdx;
                level++;
                continue;
            }
            if (this->freeTable[rightIdx]) {
                this->freeLists[level + 1] = blockForIndex(rightIdx);
                this->freeLists[level + 1]->next = nullptr;
            }
            if (this->splitTable[leftIdx]) {
                nodeIdx = leftIdx;
            } else {
                break;
            }
            level++;
        } else {
            logger.log(Logger::SeverityLevel::warning, Logger::Action::initialisation,
                       "Block is not split during filling of linked lists");
        }
    }

    available = this->workSize - leafsNeeded*MIN_ALLOC_BLOCK_SIZE;
    used = 0;
    logger.log(Logger::SeverityLevel::info, Logger::Action::none, 
               "Available for allocation: " + Utility::stringFor(available));
}


void Allocator::initTree() {
    const std::size_t dummyMemorySize = workSize - allocatedSize;
    this->workBuffer = this->allocatedBuffer - dummyMemorySize;

    const std::size_t splitTableSizeInBytes = 1 << (LEVELS - 1 - 3); // -1, because we do not split leafs, -3, because we are using bitset
    const std::size_t freeTableSizeInBytes = 1 << (LEVELS - 3 - 1); // -3, because we are using bitset, -1, because we keep two buddies in one bit
    const std::size_t freeListsSizeInBytes = LEVELS * sizeof(Block*); 

    this->splitTable.initTable((1 << (LEVELS)), this->allocatedBuffer, splitTableSizeInBytes);
    this->freeTable.initTable((1 << (LEVELS)), this->allocatedBuffer + splitTableSizeInBytes, freeTableSizeInBytes);
    this->freeLists = reinterpret_cast<Block**>(this->allocatedBuffer + splitTableSizeInBytes + freeTableSizeInBytes);
    memset(this->freeLists, 0, freeListsSizeInBytes);

    reserveDummy(dummyMemorySize + splitTableSizeInBytes + freeTableSizeInBytes + freeListsSizeInBytes);
}

std::size_t Allocator::blockLevelInTreeForSize(std::size_t size) {
    return Utility::log2(this->workSize) - Utility::log2(size);
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
    std::size_t indexOfBlock = (reinterpret_cast<std::byte*>(block) - this->workBuffer) / blockSize;
    std::size_t indexOfBuddy = indexOfBlock ^ 1;
    return reinterpret_cast<Block*>(this->workBuffer + indexOfBuddy* blockSize);
}

std::size_t Allocator::indexForBlock(Block* block, std::size_t blockSize)
{
    // index_in_level_of(p,n) == (p - _buffer_start) / size_of_level(n)
    std::size_t indexInLevel = (reinterpret_cast<std::byte*>(block) - this->workBuffer) / blockSize;
    return (1<<blockLevelInTreeForSize(blockSize)) + indexInLevel - 1;
}

std::size_t Allocator::levelIndexForBlockForTreeIndex(std::size_t treeIndex)
{
    if (treeIndex == 0) { return 0; }
    std::size_t closestBiggerPowerOf2 = Utility::closestBiggerPowerOf2(treeIndex);
    if (treeIndex == closestBiggerPowerOf2 - 1) {
        return 0;
    }
    std::size_t firstIndexInLevel = Utility::closestSmallerPowerOf2(treeIndex) - 1;
    return treeIndex - firstIndexInLevel;
}

Allocator::Block* Allocator::blockForIndex(std::size_t index)
{
    const std::size_t blockLevel = levelForIndex(index);
    const std::size_t blockSize = sizeForLevel(blockLevel);
    const std::size_t blockIndexInLevel = levelIndexForBlockForTreeIndex(index);
    return reinterpret_cast<Block*>(this->workBuffer + blockSize*blockIndexInLevel);
}

std::size_t Allocator::levelForIndex(std::size_t index)
{
    if (index == 0) { return 0; }
    std::size_t location = Utility::closestBiggerPowerOf2(index);
    if (index == location || index == location - 1) {
        return Utility::log2(location);
    }
    return Utility::log2(location) - 1;
}

std::size_t Allocator::levelForAllocatedBlock(std::byte* address)
{
    std::size_t level = LEVELS - 1;
    std::size_t idxInLevel = (address - this->workBuffer) / MIN_ALLOC_BLOCK_SIZE;
    std::size_t treeIdx = (1 << (LEVELS - 1)) - 1 + idxInLevel;

    while (level > 0) {
        std::size_t parentIdx = (treeIdx - 1) / 2;
        if (this->splitTable[parentIdx]) {
            return level;
        }
        else {
            treeIdx = parentIdx;
            level--;
        }
    }
    return 0;
}

void Allocator::deallocateTree(std::size_t treeIdx) {
    if (this->splitTable[treeIdx] == false) {
        // This entire block is allocated, not some of it's children
        Block* const rogueBlock = blockForIndex(treeIdx);
        const std::size_t rogueBlockSize = sizeForLevel(levelForIndex(treeIdx));

        logger.log(Logger::SeverityLevel::warning, Logger::Action::leak,
            "Address: " + Utility::ptrToHexStr(rogueBlock));
        logger.log(Logger::SeverityLevel::warning, Logger::Action::leak,
            "Size: " + Utility::stringFor(rogueBlockSize));

        deallocate(rogueBlock, rogueBlockSize);
    } else {
        std::size_t leftChildIdx = 2 * treeIdx + 1;
        std::size_t rightChildIdx = 2 * treeIdx + 2;
        // If both children are not free
        if (this->freeTable[leftChildIdx] == 0) { // Or right child, doesn't matter
            deallocateTree(leftChildIdx);
            deallocateTree(rightChildIdx);
        } else {
            std::size_t level = levelForIndex(leftChildIdx); // Or right child, doesn't matter
            // Check whether the node is in the free list
            Block* block = this->blockForIndex(leftChildIdx);
            Block* it = this->freeLists[level];
            bool found = false;
            while (it != nullptr) {
                if (it == block) {
                    found = true;
                    break;
                }
                it = it->next;
            }
            // It the node is in the free list, aka it is not allocated
            if (found) {
                // Deallocate its buddy
                deallocateTree(rightChildIdx);
            } else {
                deallocateTree(leftChildIdx);
            }
        }
    }
}

void Allocator::collectGarbage() {
    const std::size_t numberOfReservedLeafs = (this->workSize - this->available) / MIN_ALLOC_BLOCK_SIZE;
    const StaticString<64> mask("0" + Utility::decToBin((1 << (LEVELS - 1)) - numberOfReservedLeafs));
    // On each index the mask shows whether there must be a free block in that level; if mask[level] == 1 then there must be a free block 

    std::size_t idx = 0;
    std::size_t level = 0;
    while (level < LEVELS - 1) {
        const bool childBit = mask[level + 1] == '1';
        const std::size_t leftIdx = 2 * idx + 1;
        const std::size_t rightIdx = 2 * idx + 2;

        // If there must be a free block
        if (childBit) {
            // But the block isn't free
            if (this->freeTable[rightIdx] == 0) {
                // Free the block by deallocating all blocks in the whole subtree
                deallocateTree(rightIdx);
            }
            idx = leftIdx;
        }
        else {
            idx = rightIdx;
        }
        level++;
    }
}

bool Allocator::blockIsReserved(Block* block) {
    const std::byte* const address = reinterpret_cast<std::byte*>(block);
    return address < (this->workBuffer + (this->workSize - available));
}

void* Allocator::_allocate(std::size_t size) {
    std::size_t level = blockLevelInTreeForSize(size);

    // If a block with the correct size is available return it
    if (this->freeLists[level] != nullptr) {        
        const std::size_t idx = indexForBlock(this->freeLists[level], sizeForLevel(level));
        this->freeTable.take(idx);
        return std::exchange(this->freeLists[level], this->freeLists[level]->next);
    }

    // Go up the tree trying to find a block to split
    while (level != -1 && this->freeLists[level] == nullptr) {
        level--;
    }
    // If no memory is found
    if (level == -1) {
        throw std::bad_alloc();
    }

    do {
        // Get the block which must be split
        std::byte* blockToBeSplit = reinterpret_cast<std::byte*>(this->freeLists[level]);
        // Mark as split
        const std::size_t idx = indexForBlock(this->freeLists[level], sizeForLevel(level));

        this->splitTable.set(idx, true);
        this->freeTable.take(idx);
        // Remove it from the list for it's size
        this->freeLists[level] = this->freeLists[level]->next;
        
        // Create its two children
        Block* left = reinterpret_cast<Block*>(blockToBeSplit);
        Block* right = reinterpret_cast<Block*>(blockToBeSplit + sizeForLevel(level + 1));
        // Add them to the list for the next level
        right->next = this->freeLists[level + 1];
        left->next = right;
        this->freeLists[level + 1] = left;

        level++;
    } while (level < blockLevelInTreeForSize(size));

    const std::size_t idx = indexForBlock(this->freeLists[level], sizeForLevel(level));
    this->freeTable.take(idx);

    return std::exchange(this->freeLists[level], this->freeLists[level]->next);
}

void Allocator::_deallocate(Block* block, std::size_t size) {
    Block* buddy = buddyOf(block, size);
    std::size_t blockIdx = indexForBlock(block, size);
    std::size_t buddyIdx = indexForBlock(buddy, size);
    std::size_t level = blockLevelInTreeForSize(size);

    // Mark block as free
    this->freeTable.free(blockIdx);

    // If we have deallocated all memory(reached the root of the tree)
    if (size == this->workSize) {
        block->next = nullptr;
        this->freeLists[0] = block;
        return;
    }

    // If it's buddy is not available then we do not merge
    if (this->freeTable[buddyIdx]) {
        // Add the block in the free list
        block->next = this->freeLists[level];
        this->freeLists[level] = block;
        return;
    }
    // Buddy is available
    // Merge all free blocks and continue recursively up the tree

    // Remove buddy from the free list
    Block** it = &(this->freeLists[level]);
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
    // Unsplit parent
    this->splitTable.set((blockIdx - 1) / 2, false);
    _deallocate(mergedBlock, size);
}

Allocator::Allocator(const std::size_t size, std::ostream& logStream) : logger(logStream),
MIN_ALLOC_BLOCK_SIZE(16),
allocatedBuffer(initBuffer(size)),
workSize(Utility::closestBiggerPowerOf2(size)),
allocatedSize(size)
{
    LEVELS = Utility::log2(workSize) - Utility::log2(MIN_ALLOC_BLOCK_SIZE) + 1;
   
    logger.log(Logger::SeverityLevel::info, Logger::Action::none, 
               "Allocator constructed with size " + Utility::stringFor(size));

    logger.log(Logger::SeverityLevel::info, Logger::Action::none, 
               "Working size: " + Utility::stringFor(workSize));

    memset(this->allocatedBuffer, 0, this->allocatedSize);
    initTree();
}

Allocator::~Allocator() {
    if (this->used > 0) {
        logger.log(Logger::SeverityLevel::warning, Logger::Action::leak, "Memory leak detected");
        collectGarbage();
    }
    if (used == 0) {
        logger.log(Logger::SeverityLevel::info, Logger::Action::leak, "All memory leaks resolved");
    }
    else {
        logger.log(Logger::SeverityLevel::warning, Logger::Action::leak, "Memory leak not cleaned");
    }
    free(this->allocatedBuffer);
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
        "Block with address " + Utility::ptrToHexStr(ans) + " is allocated");
    used += size;
    logger.log(Logger::SeverityLevel::info, Logger::Action::allocation,
        "Bytes remaining for allocation: " + Utility::stringFor(available - used));

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
        "Request for deallocation of block " + Utility::ptrToHexStr(address) + " with size " + 
         Utility::stringFor(size) + " bytes");

    size = Utility::closestBiggerPowerOf2(size);
    size = std::max(size, this->MIN_ALLOC_BLOCK_SIZE);
    memset(address, 'D', size); // 'D'eallocated
    _deallocate(reinterpret_cast<Block*>(address), size);

    logger.log(Logger::SeverityLevel::info, Logger::Action::deallocation,
        "Deallocation of block " + Utility::ptrToHexStr(address) + " with size " +
        Utility::stringFor(size) + " bytes completed");
    used -= size;
    logger.log(Logger::SeverityLevel::info, Logger::Action::deallocation,
        "Bytes available for allocation: " + Utility::stringFor(available - used));
}

void Allocator::deallocate(void* address) noexcept {
    std::size_t level = levelForAllocatedBlock(reinterpret_cast<std::byte*>(address));
    std::size_t blockSize = sizeForLevel(level);
    this->deallocate(address, blockSize);
}
