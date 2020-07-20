#include "Utility.h"
#include <cmath>

unsigned long Utility::closestBiggerPowerOf2(unsigned long number) {
    if (number == 0) {
        return 1;
    }
    // https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
    if ((number & (number - 1)) == false) {
        return number;
    }
    return (1 << ((long)log2(number) + 1));
}

unsigned long Utility::closestSmallerPowerOf2(unsigned long number) {
    if (number == 0) { return 0; }
    return (unsigned int)(1 << (unsigned int)(log2(number)));
}

unsigned long Utility::log2(unsigned long number) {
    return static_cast<unsigned long>(std::log2(number));
}

StaticString<64> Utility::decToBin(unsigned long number) {
    if (number == 0) { return { '0' }; }
    StaticString<64> result;
    while (number) {
        result += static_cast<char>((number % 2) + '0');
        number /= 2;
    }
    std::reverse(result.begin(), result.end());
    return result;
}

StaticString<64> Utility::ptrToHexStr(void* ptr) {
    unsigned long value = reinterpret_cast<unsigned long>(ptr);
    StaticString<64> result;
    while (value) {
        int digit = value % 16;
        if (digit < 10) {
            result += digit + '0';
        } else {
            result += 'A' + digit - 10;
        }
        value /= 16;
    }
    for (std::size_t i = result.size(); i < 8; i++) {
        result += '0';
    }
    result += "x0";
    result.reverse();
    return result;
}
