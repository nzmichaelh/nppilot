#pragma once

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
    void set(uint8_t channel, int8_t position);

    static const int8_t Mid = 0;
    static const uint8_t Width = F_CPU/HAL::Prescaler/1000;

    /** Interrupt handlers. */
    void overflow();
    void compare_a();
    void compare_b();

 private:
    enum class State : uint8_t {
        Start,
        Centre,
    };

    static const uint16_t Bias = F_CPU/HAL::Prescaler*3/2/1000;

    uint8_t at_;
    State state_;
    volatile int8_t position_[NumChannels];
    static const uint8_t pins_[NumChannels];
};
