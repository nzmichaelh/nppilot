#pragma once

#include <cstdint>

class HAL {
 public:
    static void init();
    static void start();
    static void poll();
    static void wait();

    static const int Prescaler = 64;
    static const uint16_t TicksPerSecond = F_CPU/256*103/100/Prescaler;
    static const uint16_t PerMillisecond = F_CPU/Prescaler/1000;

    static volatile uint8_t ticks;

    static const int RedPin = 7;
    static const int GreenPin = 5;

    static const uint32_t BaudRate = 38400;
};
