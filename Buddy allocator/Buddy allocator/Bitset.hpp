#pragma once
#include <cstddef>
class Bitset
{
private:
	std::byte* const block;
	const std::size_t sizeInBytes;
public:
	Bitset(std::byte* block, std::size_t sizeInBytes);
	bool operator[](std::size_t index) const;
	Bitset& flip();
	Bitset& flip(std::size_t index);
	Bitset& set(std::size_t index, bool value);
};
