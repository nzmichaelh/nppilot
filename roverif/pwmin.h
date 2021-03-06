#pragma once

#include "hal.h"

class PWMIn {
 public:
    PWMIn();

    static const int NumChannels = 6;

    static const int8_t Missing = -128;
    static const int8_t Full = F_CPU/HAL::Prescaler/1000/2;

    void init();
    void expire();

    int8_t get(uint8_t channel) const;
    void get_all(int8_t* pinto) const;

    volatile uint8_t cycles;

    void pcint();

 private:
    struct Input {
        volatile int8_t width;
        uint8_t rose_at;
        uint8_t good;
    };

    static const int Saturate = 10;
    static const int Center = 150;

    uint8_t level_;
    Input inputs_[NumChannels];
};
