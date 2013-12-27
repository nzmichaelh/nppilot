#pragma once

#include <cstdint>
#include "hal.h"

class Servos
{
public:
    Servos();

    void init();

    void set(uint8_t channel, uint8_t position);

    void overflow();
    void compare_a();
    void compare_b();

    const uint8_t Low = F_CPU/HAL::Prescaler/1000*1;
    const uint8_t Mid = F_CPU/HAL::Prescaler/1000*3/2;
    const uint8_t High = F_CPU/HAL::Prescaler/1000*2;

private:
    static const int NumChannels = 5;

    uint8_t at_;
    volatile uint8_t position_[NumChannels];
    static const uint8_t pins_[NumChannels];
};
