
#pragma once

#include <stdint.h>

class PWMIn
{
public:
    static const uint16_t Missing = 0;

    PWMIn();

    int count() const { return Count; }
    uint16_t value(int channel) const { return value_[channel]; }

    void irq(int channel, uint32_t ccr, volatile uint32_t* ccer, int mask);
    void expire();

private:
    static const int Count = 6;

    uint16_t value_[Count];
    uint16_t last_[Count];
    uint8_t recent_;
};
