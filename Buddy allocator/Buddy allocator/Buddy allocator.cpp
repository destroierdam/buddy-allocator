#include <iostream>
#include <fstream>
#include <cstring>
#include "Allocator.hpp"
#include "Utility.h"
#include "StaticString.h"
#include "Bitset.h"
#include <cassert>

struct tripleLong {
    long left;
    long mid;
    long right;
};

void testUtilityReverseNumber() {
    std::size_t number = 123456;
    StaticString<64> str = Utility::stringFor(number);
    assert(str == "654321");
}

void testDummyAlloc() {
    Allocator a(95);
    char* str = static_cast<char*>(a.allocate(64));
    strcpy_s(str, 64, "The quick brown fox jumps over the lazy dog. Make them 64 chars");

    char* str2 = static_cast<char*>(a.allocate(16));
    strcpy_s(str2, 16, "Str w Len16char");

}

void testBitset() {
    std::size_t size = 32;
    std::byte* memory = static_cast<std::byte*>(malloc(size));
    Bitset bitset(memory, size);
    assert(bitset[0] == false);
    assert(bitset[7] == false);
    assert(bitset[30] == false);

    bitset.set(12, true);
    assert(bitset[12]);
    assert(bitset[11] == false);
    assert(bitset[13] == false);

    bitset.flip(12);
    assert(bitset[12] == false);
    assert(bitset[11] == false);
    assert(bitset[13] == false);

    bitset.set(0, true);
    assert(bitset[0]);
    assert(bitset[1] == false);
    assert(bitset[2] == false);
    
    for (std::size_t i = 1; i <= 32; i++) {
        bitset.set(i, true);
        assert(bitset[i-1]);
        assert(bitset[i]);
        assert(bitset[i+1] == false);
    }

    bitset.set(0, false);
    assert(bitset[0] == false);
    assert(bitset[1] == true);
    assert(bitset[2] == true);

    for (std::size_t i = 1; i < 32; i++) {
        bitset.set(i, false);
        assert(bitset[i - 1] == false);
        assert(bitset[i] == false);
        assert(bitset[i + 1]);
    }

    assert(bitset[33] == false);
    
}

void testUtilitySmallerPower() {
    assert(Utility::closestSmallerPowerOf2(7) == 4);
    assert(Utility::closestSmallerPowerOf2(14) == 8);
    assert(Utility::closestSmallerPowerOf2(8) == 8);
}

void runTests() {
    testUtilityReverseNumber();
    testUtilitySmallerPower();
    testDummyAlloc();
    testBitset();
}

int main()
{
    runTests();
    std::ofstream logStream("Log.csv");
    Allocator allocator(128);

    tripleLong * n = static_cast<tripleLong*>(allocator.allocate(sizeof(tripleLong)));
    n->left = 'A';
    n->mid = 255;
    n->right = 256;

    long* ptr = static_cast<long*>(allocator.allocate(sizeof(long)));
    *ptr = 31;

    char* str = static_cast<char*>(allocator.allocate(64));
    strcpy_s(str, 64, "The quick brown fox jumps over the lazy dog. Make them 64 chars");

    allocator.deallocate(n, sizeof(tripleLong));
    allocator.deallocate(ptr, sizeof(long));
    allocator.deallocate(str, 64);
    
    char* newString = static_cast<char*>(allocator.allocate(64));
    strcpy_s(newString, 64, "Another string with length 64 bytes used for testing here!!!!!!");

    n = static_cast<tripleLong*>(allocator.allocate(sizeof(tripleLong)));
    void* badAllocEx = allocator.allocate(64, std::nothrow);
    assert(badAllocEx == nullptr);
}
