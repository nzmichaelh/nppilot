#include "timer.h"
#include "switcher.h"

/* The linker rounds the section up to the nearest four bytes */
static_assert(sizeof(Timer::Fixed) % 4 == 0, "Timer::Fixed must be a multiple of four");

void Timer::tick(const Timer::Fixed& fixed)
{
    if (_remain >= Reserved) {
        /* Nothing to do */
    } else if (_remain == 0) {
        _remain = fixed.period;

        if (fixed.id >= 0) {
            dispatch(fixed.id);
        }
    }
    else {
        _remain--;
    }
}
