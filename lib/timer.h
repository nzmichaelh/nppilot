/*
 * Periodic or single shot timer.
 */

#pragma once

#include "hal.h"
#include <cstdint>

/**
 * Periodic or single shot timer.
 *
 * Periodic timers start on program start and fire at a fixed
 * interval.  Single shot timers have a reload of `Stopped` and are
 * started by calling start(remain).
 *
 * Timers can trigger a thread.  Use -1 to create a detached timer.
 */
class Timer {
 public:
    /** Special value for a stopped timer. */
    static const uint8_t Stopped = 0xFF;

    /** Stop the timer running. */
    void stop() { _remain = Stopped; }
    /** 
     * Start the timer running.  Fires in `remain` ms.  Reloads with
     * the timers initial reload value.
     */
    void start(int remain_ms) { _remain = to_ticks(remain_ms); }

    /** Returns true if the timer is currently running. */
    bool running() const { return _remain < Reserved; }

    /** Tick this timer, reloading and returning true if expired. */
    bool tick() { return tick_internal(); }
    bool tick(int reload_ms) { return tick_internal(to_ticks(reload_ms)); }

 private:
    static const uint8_t Reserved = 0xFE;

    uint8_t to_ticks(int ms) {
        return (HAL::TicksPerSecond * ms + 500) / 1000;
    }

    bool tick_internal(uint8_t reload = Stopped);

    uint8_t _remain;
};
