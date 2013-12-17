#include <cstdint>

class HAL
{
public:
    static void init();
    static void start();
    static void poll();
    static void wait();

    static const uint16_t TicksPerSecond = F_CPU/256/256;

    static volatile uint8_t ticks;

    static const int RedPin = 6;
    static const int GreenPin = 7;
};
