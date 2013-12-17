#include "hal.h"
#include <blinker.h>
#include <avr/io.h>

volatile uint8_t HAL::ticks;

void HAL::init()
{
    DDRD |= 0
        | _BV(RedPin)
        | _BV(GreenPin)
        ;
}

void HAL::start()
{
}

void HAL::poll()
{
}

void HAL::wait()
{
}

void Blinker::update(bool red_on, bool green_on)
{
    if (red_on) {
        PORTD |= _BV(HAL::RedPin);
    } else {
        PORTD &= ~_BV(HAL::RedPin);
    }

    if (green_on) {
        PORTD |= _BV(HAL::GreenPin);
    } else {
        PORTD &= ~_BV(HAL::GreenPin);
    }
}
