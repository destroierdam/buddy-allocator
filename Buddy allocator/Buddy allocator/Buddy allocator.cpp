#include <iostream>
#include <fstream>
#include <cstring>
#include "Allocator.hpp"
#include "Utility.h"
#include "StaticString.h"
#include "Bitset.h"
#include <cassert>

void testUtilityStringForNumber() {
    std::size_t number = 123456;
    StaticString<64> str = Utility::stringFor(number);
    assert(str == "123456");
}

void testDummyAlloc() {
    Allocator a(95);
    char* str = static_cast<char*>(a.allocate(64));
    strcpy_s(str, 64, "The quick brown fox jumps over the lazy dog. Make them 64 chars");
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

void testUtilityDecToBin() {
    assert(Utility::decToBin(0) == StaticString<64>('0'));
    assert(Utility::decToBin(1) == StaticString<64>('1'));
    assert(Utility::decToBin(2) == StaticString<64>("10"));
    assert(Utility::decToBin(3) == StaticString<64>("11"));
    assert(Utility::decToBin(4) == StaticString<64>("100"));
    assert(Utility::decToBin(12) == StaticString<64>("1100"));
    assert(Utility::decToBin(128) == StaticString<64>("10000000"));
    assert(Utility::decToBin(130) == StaticString<64>("10000010"));
}

void runTests() {
    testUtilityStringForNumber();
    testUtilitySmallerPower();
    testUtilityDecToBin();
    testBitset();
    testDummyAlloc();
}

int main()
{
    Allocator allocator(128);

    char * n = static_cast<char*>(allocator.allocate(16));
    strcpy_s(n, 16, "Sixteen bytes!!");

    char* str = static_cast<char*>(allocator.allocate(64));
    strcpy_s(str, 64, "The quick brown fox jumps over the lazy dog. Make them 64 chars");

    char* n2 = static_cast<char*>(allocator.allocate(16));
    strcpy_s(n2, 16, "Sixteen bytes@@");

    /*allocator.deallocate(n);
    allocator.deallocate(str);*/
    
    /*char* newString = static_cast<char*>(allocator.allocate(64));
    strcpy_s(newString, 64, "Another string with length 64 bytes used for testing here!!!!!!");

    n = static_cast<char*>(allocator.allocate(12));
    void* badAllocEx = allocator.allocate(64, std::nothrow);
    assert(badAllocEx == nullptr);*/
}
