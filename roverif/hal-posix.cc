#include "roverif.h"
#include "hal.h"
#include "board.h"

#include <cstdio>
#include <time.h>

static struct timespec _epoch;
static int64_t _ticks;

void HAL::init()
{
}

void HAL::start()
{
    clock_gettime(CLOCK_MONOTONIC, &_epoch);
}

void HAL::poll()
{
}

void HAL::wait()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    int64_t elapsed = (now.tv_sec - _epoch.tv_sec) * Board::Ticks + (now.tv_nsec - _epoch.tv_nsec) / (1000000000 / Board::Ticks);

    if (elapsed == _ticks) {
        struct timespec sleep_for = { 0, 1000000 };
        nanosleep(&sleep_for, NULL);
    } else {
        _ticks++;
        RoverIf::switcher.trigger(SysTickID);
        fflush(stdout);
    }
}

void HAL::set_status_led(bool on)
{
    if (on) {
        ::putchar('*');
    } else {
        ::putchar('.');
    }
}

void Debug::putch(char ch)
{
    ::putchar(ch);
}

int main()
{
    RoverIf::init();
    RoverIf::run();

    return 0;
}
