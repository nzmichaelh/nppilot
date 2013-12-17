#include "servos.h"
#include "hal.h"
#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t Servos::at_;
volatile uint8_t Servos::position_[NumChannels];

const uint8_t Servos::pins_[] = {
    _BV(1), _BV(2)
};

void Servos::init()
{
    for (uint8_t pin : pins_) {
        DDRB |= pin;
    }

    TCCR0A = 0
        | (0 << COM0A0)
        | (0 << COM0B0)
        | (0 << WGM00)
        ;

    static_assert(Prescaler == 256, "Prescaler disagrees.");
    TCCR0B = 0
        | (0 << WGM02)
        | (4 << CS00)
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
    at_ = at;
    uint8_t position = position_[at];
    uint8_t offset = 128 - position/2;
    OCR0A = offset;
    OCR0B = offset + position;
}

inline void Servos::compare_a()
{
    PINB = pins_[at_];
}

inline void Servos::compare_b()
{
    PINB = pins_[at_];
}

ISR(TIMER0_OVF_vect)
{
    Servos::overflow();
    HAL::ticks++;
}

ISR(TIMER0_COMPA_vect)
{
    Servos::compare_a();
}

ISR(TIMER0_COMPB_vect)
{
    Servos::compare_b();
}
