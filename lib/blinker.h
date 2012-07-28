/*
 * Blinks the status LED.
 */

#pragma once

#include <cstdint>

/**
 * Blinks the status LED.  Scans out the bit pattern LSB to MSB and
 * stops when there's one set bit remaining.  Use to set the period.
 * New values take affect after reload.
 */
class Blinker
{
public:
    Blinker();

    /**
     * Update the pattern.  Starts at next reload.  Restarts on
     * hitting the highest set bit.
     *
     * Examples:
     *  0b101: on, off
     *  0b1000001: on, off for five
     *  0b1000101: on, off, on, off for three
     */
    void set(uint16_t pattern) { reload_ = pattern; }

    /** Called on a periodic 100 ms timer. */
    void tick();

private:
    void update(bool is_on);

    uint16_t pattern_;
    uint16_t reload_;
};
