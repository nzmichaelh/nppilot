#pragma once

#include <cstdint>

struct Timer
{
public:
    struct Fixed
    {
        uint16_t period;
        int16_t id;
    };

    static const uint16_t Stopped = 0xFFFF;

    void stop() { _remain = Stopped; }
    void start(int remain) { _remain = remain; }
    void tick(const Fixed& fixed);

    static void tick_all();

private:
    static const uint16_t Reserved = 0xFFFE;

    uint16_t _remain;
};

#define MAKE_TIMER(_name, _id, _period)          \
    Timer timer_##_name __attribute__((section(".timers")));     \
    Timer::Fixed timer_fixed_##_name                      \
    __attribute__((section(".timers.ro"))) \
                       = { _period - 1, _id };
