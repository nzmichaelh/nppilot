#include "switch.h"

#include <algorithm>
#include <cstdio>

Switch::Switch()
    : position_(-1)
{
}

int Switch::update(const Fixed& fixed, uint16_t level)
{
    int lowest = UINT16_MAX;
    int next = -1;

    for (int i = 0; i < 4; i++) {
        int delta = std::abs(level - fixed.levels[i]);

        if (i == position_) {
            /* Boost the last position */
            delta -= Hysteresis;
        }

        if (delta < lowest) {
            next = i;
            lowest = delta;
        }
    }

    position_ = next;
    return next;
}
