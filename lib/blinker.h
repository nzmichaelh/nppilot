/*
 * Blinks the status LED.
 */

#pragma once

#include <cstdint>

/**
 * Blinks the status LEDs.  Scans out the bit pattern LSB to MSB and
 * stops when there's one set bit remaining.  Use to set the period.
 * New values take affect after reload.
 */
class Blinker {
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
    void set(uint8_t red, uint8_t green) {
        red_reload_ = red;
        green_reload_ = green;
    }

    /** Called on a periodic 100 ms timer. */
    void tick();

 private:
    void update(bool red_on, bool green_on);

    uint8_t red_;
    uint8_t red_reload_;
    uint8_t green_;
    uint8_t green_reload_;
};
