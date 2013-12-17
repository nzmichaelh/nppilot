/*
 * Periodic or single shot timer.
 */

#pragma once

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
class Timer
{
public:
    /** Special value for a stopped timer. */
    static const uint8_t Stopped = 0xFF;

    /** Stop the timer running. */
    void stop() { _remain = Stopped; }
    /** 
     * Start the timer running.  Fires in `remain` ms.  Reloads with
     * the timers initial reload value.
     */
    void start(int remain) { _remain = remain; }

    /** Returns true if the timer is currently running. */
    bool running() const { return _remain < Reserved; }

    /** Tick this timer, reloading and returning true if expired. */
    bool tick(uint8_t reload = Stopped);

private:
    static const uint8_t Reserved = 0xFE;
    uint8_t _remain;
};
