/*
 * Calls run to completion threads in priority order.
 */

#pragma once

#include <bitarray.h>

/**
 * Calls run to completion threads in priority order.
 *
 * Higher thread IDs are higher priority.  Invokes the highest
 * priority thread, then the next, until all ready threads have run.
 * Threads may trigger others or themselves.
 */
class Switcher
{
public:
    /** Special value for no thread */
    static const int None = -1;

    Switcher();

    /** Trigger a thread for later execution. */
    void trigger(int id);
    /** Runs all ready threads until completion. */
    void next();

private:
    typedef void (*thread_entry)(void);

    BitArray _running;
    BitArray _pending;

    static const thread_entry _dispatch[];
};

extern Switcher switcher;
