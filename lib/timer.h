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
struct Timer
{
public:
    struct Fixed
    {
        uint16_t period;
        int16_t id;
    };

    /** Special value for a stopped timer. */
    static const uint16_t Stopped = 0xFFFF;

    /** Stop the timer running. */
    void stop() { _remain = Stopped; }
    /** 
     * Start the timer running.  Fires in `remain` ms.  Reloads with
     * the timers initial reload value.
     */
    void start(int remain) { _remain = remain; }

    /** Returns true if the timer is currently running. */
    bool running() const { return _remain < Reserved; }

    /** Ticks all system timers, firing those that elapse. */
    static void tick_all();

private:
    static const uint16_t Reserved = 0xFFFE;

    void tick(const Fixed& fixed);
    void dispatch(int id);

    uint16_t _remain;
};

/**
 * Make a timer called `_name` that triggers the thread `_id` and
 * has the period `_period`.  See Switcher::None and
 * Timer::Stopped for detached and single shot timers.
 */
#define MAKE_TIMER(_name, _id, _period)               \
    Timer _name __attribute__((section(".timers")));  \
    Timer::Fixed _name##_fixed                        \
    __attribute__((section(".timers.ro"))) \
                       = { _period - 1, _id };
