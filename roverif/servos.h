#pragma once

#include <cstdint>
#include "hal.h"

class Servos
{
public:
    static void init();

    static void set(uint8_t channel, uint8_t position);

    static void overflow();
    static void compare_a();
    static void compare_b();

    static const uint8_t Low = F_CPU/HAL::Prescaler/1000*1;
    static const uint8_t Mid = F_CPU/HAL::Prescaler/1000*3/2;
    static const uint8_t High = F_CPU/HAL::Prescaler/1000*2;

private:
    static const int NumChannels = 5;

    static uint8_t at_;
    static volatile uint8_t position_[NumChannels];
    static const uint8_t pins_[NumChannels];
};
