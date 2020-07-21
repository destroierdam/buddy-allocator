#include <iostream>
#include <fstream>
#include <cstring>
#include "Allocator.hpp"
#include "Utility.hpp"
#include "StaticString.hpp"

void testDummyAlloc() {
    Allocator a(95);
    char* str = static_cast<char*>(a.allocate(64));
    strcpy_s(str, 64, "The quick brown fox jumps over the lazy dog. Make them 64 chars");
}

void gcTwoReservedLeafs() {
    Allocator a(128);
    char* idx9 = static_cast<char*>(a.allocate(16));
    strcpy_s(idx9, 16, "Index 9!16 chs!");

    char* idx10 = static_cast<char*>(a.allocate(16));
    strcpy_s(idx10, 16, "Index10:16 chs!");

    char* idx2 = static_cast<char*>(a.allocate(64));
    strcpy_s(idx2, 64, "The quick brown fox jumps over the lazy dog. Make them 64 chars");
}
void gcThreeReservedLeafs() {
    Allocator a(113); 

    char* idx10 = static_cast<char*>(a.allocate(16));
    strcpy_s(idx10, 16, "Index10!16 chs!");

    char* idx5 = static_cast<char*>(a.allocate(32));
    strcpy_s(idx5, 32, "Index 5!32 chs!");

    char* idx13 = static_cast<char*>(a.allocate(16));
    strcpy_s(idx13, 16, "Index13!16 chs!");

    char* idx14 = static_cast<char*>(a.allocate(16));
    strcpy_s(idx14, 16, "Index14!16 chs!");

    a.deallocate(idx13);


}
void testGarbageCollection() {
    gcTwoReservedLeafs();
    gcThreeReservedLeafs();
}

void runTests() {
    testDummyAlloc();
    testGarbageCollection();
}

int main()
{
    // runTests();
    Allocator allocator(129);

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
