#include "timer.h"

/* The linker rounds the section up to the nearest four bytes */
static_assert(sizeof(Timer::Fixed) % 4 == 0, "Timer::Fixed must be a multiple of four");

extern Timer __timers_start;
extern Timer __timers_end;
extern const Timer::Fixed __timers_ro_start;
extern const Timer::Fixed __timers_ro_end;

void Timer::tick_all()
{
    const Timer::Fixed* pfixed = &__timers_ro_start;
    Timer* ptimer = &__timers_start;

    for (; pfixed < &__timers_ro_end; pfixed++, ptimer++) {
        ptimer->tick(*pfixed);
    }
}
