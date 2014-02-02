#pragma once

#include <cstdint>

class HAL {
 public:
    static void init();
    static void start();
    static void poll();
    static void wait();

    static const int Prescaler = 64;
    static const int TickPrescaler = 4;
    static const uint16_t Timer0PerMillisecond = F_CPU/Prescaler/1000;
    static const uint16_t TicksPerSecond = F_CPU/256/Prescaler/TickPrescaler;

    static volatile uint8_t ticks;

    static const int RedPin = 7;
    static const int GreenPin = 5;

    static const uint32_t BaudRate = 57600;
};
