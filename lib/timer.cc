#include "timer.h"

bool Timer::tick(uint8_t reload)
{
    if (_remain >= Reserved) {
        /* Nothing to do */
        return false;
    } else if (_remain == 0) {
        _remain = reload;
        return true;
    } else {
        _remain--;
        return false;
    }
}
