#pragma once
#include <cstddef>
#include <cmath>
#include <algorithm>
#include "StaticString.hpp"

class Utility {
public:
    static unsigned long closestBiggerPowerOf2(unsigned long number);
    static unsigned long closestSmallerPowerOf2(unsigned long number);
    template<typename T>
    static StaticString<64> stringFor(T number);
    static unsigned long log2(unsigned long number);
    static StaticString<64> decToBin(unsigned long number);
    static StaticString<64> ptrToHexStr(void* ptr);
};

template<typename T>
StaticString<64> Utility::stringFor(T number) {
    if (number == 0) { return { '0' }; }
    StaticString<64> result;
    while (number) {
        char c = number % 10 + '0';
        result += c;
        number /= 10;
    }
    if (number < 0) {
        result += '-';
    }
    std::reverse(result.begin(), result.end());
    return result;
}
