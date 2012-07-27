/*
 * Blinks the status LED.
 */

#pragma once

#include <cstdint>

/**
 * Blinks the status LED.  Scans out the bit pattern supplied where 1
 * is LED on and a 0 is LED off.  Reloads after every 16 bits.  New
 * values take affect after reload.
 */
class Blinker
{
public:
    Blinker();

    /** Update the pattern.  Starts at next reload. */
    void set(uint16_t pattern) { reload_ = pattern; }

    /** Called on a periodic 100 ms timer. */
    void tick();

private:
    void update(bool is_on);

    uint16_t pattern_;
    uint16_t reload_;
    uint8_t at_;
};
