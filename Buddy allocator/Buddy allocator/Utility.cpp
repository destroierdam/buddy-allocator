#include "Utility.h"

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
