#include "servos.h"
#include "hal.h"
#include "roverif.h"
#include <avr/io.h>
#include <avr/interrupt.h>

const uint8_t Servos::pins_[] = {
    _BV(0), _BV(1), _BV(2), _BV(3), _BV(4),
};

Servos::Servos()
{
    for (volatile uint8_t& position : position_) {
        position = Mid;
    }
}

void Servos::init()
{
    for (uint8_t pin : pins_) {
        DDRC |= pin;
    }

    TCCR0A = 0
        | (0 << COM0A0)
        | (0 << COM0B0)
        | (0 << WGM00)
        ;

    static_assert(HAL::Prescaler == 64, "Prescaler disagrees.");
    TCCR0B = 0
        | (0 << WGM02)
        | (3 << CS00)
        ;

    TIMSK0 = _BV(OCIE0A) | _BV(OCIE0B) | _BV(TOIE0);
}

void Servos::set(uint8_t channel, uint8_t position)
{
    position_[channel] = position;
}

inline void Servos::overflow()
{
    uint8_t at = at_ + 1;
    if (at == NumChannels) {
        at = 0;
    }

    uint8_t position = position_[at];
    uint8_t offset = 128 - position/2;
    OCR0A = offset;
    OCR0B = offset + position;

    at_ = at;
}

inline void Servos::compare_a()
{
    PINC = pins_[at_];
}

inline void Servos::compare_b()
{
    PINC = pins_[at_];
}

ISR(TIMER0_OVF_vect)
{
    RoverIf::servos.overflow();
    HAL::ticks++;
}

ISR(TIMER0_COMPA_vect)
{
    RoverIf::servos.compare_a();
}

ISR(TIMER0_COMPB_vect)
{
    RoverIf::servos.compare_b();
}
