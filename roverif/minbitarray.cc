#include "minbitarray.h"

int8_t MinBitArray::pop()
{
    uint8_t i = 0;

    for (uint8_t mask = 1; mask != 0; mask <<= 1, i++) {
        if ((bits_ & mask) != 0) {
            bits_ ^= mask;
            return i;
        }
    }

    return -1;
}
