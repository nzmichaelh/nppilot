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

private:
    static const int NumChannels = 2;

    static uint8_t at_;
    static volatile uint8_t position_[NumChannels];
    static const uint8_t pins_[NumChannels];
};
