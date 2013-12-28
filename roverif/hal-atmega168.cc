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

    UCSR0A = 0
        | _BV(U2X0) // Halve the divisor.
        ;
    UCSR0B = 0
        | _BV(RXCIE0)
        | _BV(TXCIE0)
//        | _BV(UDRIE0)
        | _BV(RXEN0)
        | _BV(TXEN0)
        ;
    UCSR0C = 0
        | (0 << UMSEL00)
        | (0 << UPM00)
        | (0 << USBS0)
        | (3 << UCSZ00)
        ;

    const uint32_t brr = (F_CPU / 8 / BaudRate) - 1;
    UBRR0H = (uint8_t)(brr >> 8);
    UBRR0L = (uint8_t)(brr >> 0);
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
