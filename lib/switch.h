/*
 * Analogue multi-position switch with hysteresis.
 */

#pragma once

#include <cstdint>

/**
 * Analogue multi-position switch with hysteresis.
 */
class Switch
{
public:
    struct Fixed
    {
        /**
         * The mid point for each position.  Returns the closest.
         * Add fake out of range values at zero and N-1 for out of
         * range detection.
         */
        int16_t levels[4];
    };

    Switch();

    /** Update the switch level, returning the current position. */
    int update(const Fixed& fixed, uint16_t level);

    /** Returns the current switch position. */
    int position() const { return position_; }

private:
    static const int Hysteresis = 200;

    int8_t position_;
};
