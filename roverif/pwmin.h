#pragma once

#include <cstdint>

class PWMIn
{
public:
    PWMIn();

    void init();
    void expire();

    int8_t get(uint8_t channel) const;

    void pcint();

private:
    struct Input
    {
        volatile int8_t width;
        uint8_t rose_at;
        uint8_t good;
        uint8_t pin;
    };

    static const int NumChannels = 6;
    static const int Saturate = 10;
    static const int Center = 158;

    uint8_t level_;
    Input inputs_[NumChannels];
};
