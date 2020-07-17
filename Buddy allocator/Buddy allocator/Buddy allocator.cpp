#include <iostream>
#include <fstream>
#include <cstring>
#include "Allocator.hpp"
#include "Utility.h"
#include "StaticString.h"
#include <cassert>

struct tripleLong {
    long left;
    long mid;
    long right;
};

void testUtility() {
    std::size_t number = 123456;
    StaticString<64> str = Utility::stringFor(number);
    assert(str == "654321");
}

int main()
{
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
