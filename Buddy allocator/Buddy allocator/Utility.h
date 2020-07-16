#pragma once
#include <cstddef>
#include <cmath>
#include <algorithm>
#include "StaticString.h"

class Utility {
public:
    static unsigned long closestBiggerPowerOf2(unsigned long number);
    template<typename T>
    static StaticString<64> stringFor(T number);
};

template<typename T>
StaticString<64> Utility::stringFor(T number) {
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
