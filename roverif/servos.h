#pragma once

#include <cstdint>
#include "hal.h"

/** Drives a bunch of servos.

    Generates a centered PWM signal of 1.0 to 2.0 ms plus a bit of
    margin.
*/
class Servos {
 public:
    static const int NumChannels = 6;

    Servos();

    /** Initialise and start the hardware. */
    void init();

    /** Update the width of one channel.  See Low, Mid, and High for
        some of the limits.
    */
    void set(uint8_t channel, uint8_t position);

    static const uint8_t Low = 100*F_CPU/HAL::Prescaler/1000/100;
    static const uint8_t Mid = 150*F_CPU/HAL::Prescaler/1000/100;
    // TODO(michaelh): hack to handle the 8.3 MHz clock.
    static const uint8_t High = 196*F_CPU/HAL::Prescaler/1000/100;

    /** Interrupt handlers. */
    void overflow();
    void compare_a();
    void compare_b();

 private:
    enum class State : uint8_t {
        Start,
        Centre,
    };

    /** How much to offset the second channel. */
    static const int Offset = Low/3;

    uint8_t at_;
    State state_;
    volatile uint8_t position_[NumChannels];
    static const uint8_t pins_[NumChannels];
};
