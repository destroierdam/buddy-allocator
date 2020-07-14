#include <iostream>
#include <cstring>
#include "Allocator.h"

struct tripleLong {
    long left;
    long mid;
    long right;
};
int main()
{
    Allocator allocator(128);
    tripleLong * n = static_cast<tripleLong*>(allocator.allocate(sizeof(tripleLong)));
    n->left = 'A';
    n->mid = 255;
    n->right = 256;

    long* ptr = static_cast<long*>(allocator.allocate(sizeof(long)));
    *ptr = 31;

    char* str = static_cast<char*>(allocator.allocate(64));
    strcpy_s(str, 64, "The quick brown fox jumps over the lazy dog. Make them 64 chars");
}
