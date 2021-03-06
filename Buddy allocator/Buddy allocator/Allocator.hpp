#pragma once
#include <cstddef>
#include <array>
#include <bitset>
#include <cassert>
#include "Logger.hpp"
#include "Bitset.hpp"

class Allocator {
private:
	Logger logger;

	/// The number of bytes available for allocation
	std::size_t available;

	/// The number of bytes allocated by the allocator
	std::size_t used;

	struct Block {
		Block* next;
	};

	/// Returns the index of the block in the tree which corresponds to the block size
	std::size_t blockLevelInTreeForSize(std::size_t size);

	/// Given the level in the tree calculates the size of the blocks on that level
	std::size_t sizeForLevel(std::size_t level);

	/// Returns a pointer to the buddy block based on the block's address and size
	Block* buddyOf(Block* block, std::size_t size);

	/// Returns the index of the block in a binary tree
	std::size_t indexForBlock(Block* block, std::size_t size);

	/// Given an index in a binary tree returns the index in the level
	std::size_t levelIndexForBlockForTreeIndex(std::size_t treeIndex);

	/// Returns the block from an index in a binary tree
	Block* blockForIndex(std::size_t index);

	/// Returns the level in the tree in which the index is 
	std::size_t levelForIndex(std::size_t index);

	/// Given an address of an allocated block returns the level in the tree
	std::size_t levelForAllocatedBlock(std::byte* address);

	/// Given an index of a block deallocates all allocated blocks in it
	void deallocateTree(std::size_t treeIdx);

	/// Finds all allocated blocks of memory, logs them and deallocates them
	void collectGarbage();

	/// Returns true if the block is reserved for use for the allocator
	bool blockIsReserved(Block* block);

	class SplitTable {
	private:
		std::optional<Bitset> table;
		std::size_t sizeOfEntireTree;
	public:
		SplitTable() :table(std::nullopt), sizeOfEntireTree(0) {}
		void initTable(std::size_t sizeOfEntireTree, std::byte* buffer, std::size_t sizeInBytes) {
			this->sizeOfEntireTree = sizeOfEntireTree;
			this->table.emplace(buffer, sizeInBytes);
		}

		bool operator[](std::size_t index) const {
			if (index < sizeOfEntireTree / 2 - 1) {
				return table.value()[index];
			}
			else {
				return false;
			}
		}
		SplitTable& flip() {
			table.value().flip();
			return *this;
		}
		SplitTable& flip(std::size_t index) {
			if (index < sizeOfEntireTree / 2) {
				table.value().flip(index);
			}
			else {
				assert(false);
			}
			return *this;
		}
		SplitTable& set(std::size_t index, bool value) {
			if (index < sizeOfEntireTree / 2) {
				table.value().set(index, value);
			}
			else {
				assert(false);
			}
			return *this;
		}
	};

	SplitTable splitTable;


	class FreeTable {
	private:
		bool root : 1;
		std::optional<Bitset> table;
		std::size_t sizeOfEntireTree;
	public:
		FreeTable() :root(false), table(std::nullopt), sizeOfEntireTree(0) {}
		void initTable(std::size_t sizeOfEntireTree, std::byte* buffer, std::size_t sizeInBytes) {
			this->sizeOfEntireTree = sizeOfEntireTree;
			this->table.emplace(buffer, sizeInBytes);
		}
		bool operator[](std::size_t index) {
			if (index == 0) { return root; }
			const std::size_t offsetIndex = (index - 1) / 2;
			return table.value()[offsetIndex];
		}
		void flip(const std::size_t index) {
			if (index == 0) { this->root = !(this->root); return; }
			const std::size_t offsetIndex = (index - 1) / 2;
			this->table.value().flip(offsetIndex);
		}
		void free(const std::size_t index) {
			if (index == 0) { this->root = 1; return; }
			this->flip(index);
		}
		void take(const std::size_t index) {
			if (index == 0) { this->root = 0; return; }
			this->flip(index);
		}
	};

	FreeTable freeTable;

	/// The minimum size of the blocks that can be allocated
	const std::size_t MIN_ALLOC_BLOCK_SIZE;

	/// The depth of the tree
	std::size_t LEVELS;

	/// An array of linked lists containing the free blocks
	Block** freeLists;

	/// The theoretical beginning address of the buffer
	std::byte* workBuffer;

	/// The address of the buffer actually allocated from the OS
	std::byte* const allocatedBuffer;

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
	void _deallocate(Block* address, const std::size_t size);

public:
	Allocator(const std::size_t size, std::ostream& logStream = std::clog);
	Allocator(const Allocator& other) = delete;
	Allocator& operator=(const Allocator& other) = delete;
	~Allocator();

	void* allocate(std::size_t size);
	void* allocate(std::size_t size, std::nothrow_t) noexcept;
	void deallocate(void* address, const std::size_t size) noexcept;
	void deallocate(void* address) noexcept;
};
