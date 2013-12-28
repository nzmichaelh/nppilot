#pragma once

#include <cstdint>
#include "hal.h"

/** Drives a bunch of servos.

    Generates a centered PWM signal of 1.0 to 2.0 ms plus a bit of
    margin.
*/
class Servos {
 public:
    Servos();

    /** Initialise and start the hardware. */
    void init();

    /** Update the width of one channel.  See Low, Mid, and High for
        some of the limits.
    */
    void set(uint8_t channel, uint8_t position);

    static const uint8_t Low = F_CPU/HAL::Prescaler/1000*1;
    static const uint8_t Mid = F_CPU/HAL::Prescaler/1000*3/2;
    static const uint8_t High = F_CPU/HAL::Prescaler/1000*2;

    /** Interrupt handlers. */
    void overflow();
    void compare_a();
    void compare_b();

 private:
    enum class State : uint8_t {
        Start,
        Centre,
    };

    static const int NumChannels = 6;
    /** How much to offset the second channel. */
    static const int Offset = Low/3;

    uint8_t at_;
    State state_;
    volatile uint8_t position_[NumChannels];
    static const uint8_t pins_[NumChannels];
};
