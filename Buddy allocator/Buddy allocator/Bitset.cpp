#include "Bitset.h"
#include <climits>
#include <cstring>

Bitset::Bitset(std::byte* block, std::size_t sizeInBytes):block(block), sizeInBytes(sizeInBytes) {
	memset(this->block, 0, sizeInBytes);
}

bool Bitset::operator[](const std::size_t index) const {
	const std::size_t byteIndex = index / CHAR_BIT;
	const std::size_t bitIndex = index % CHAR_BIT;
	const bool res = (bool)(this->block[byteIndex] & (std::byte)(1 << (CHAR_BIT - bitIndex-1)));
	return res;
}

Bitset& Bitset::flip() {
	for (std::size_t i = 0; i < this->sizeInBytes; i++) {
		this->block[i] = ~(this->block[i]);
	}
	return *this;
}

Bitset& Bitset::flip(std::size_t index) {
	const std::size_t byteIndex = index / CHAR_BIT;
	const std::size_t bitIndex = index % CHAR_BIT;
	this->block[byteIndex] ^= static_cast<std::byte>(1UL << (CHAR_BIT - bitIndex-1));
	return *this;
}

Bitset& Bitset::set(std::size_t index, bool value) {
	const std::size_t byteIndex = index / CHAR_BIT;
	const std::size_t bitIndex = index % CHAR_BIT;
	if (value) {
		this->block[byteIndex] |= static_cast<std::byte>(1UL << (CHAR_BIT - bitIndex-1));
	} else {
		this->block[byteIndex] &= ~(static_cast<std::byte>(1UL << (CHAR_BIT - bitIndex-1)));
	}
	
	return *this;
}
