#include "servos.h"
#include "hal.h"
#include "roverif.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* The timer runs pretty fast to get good enough resolution.  The
   prescaler has limited steps, meaning that a 2.0 ms pulse gets very
   close to overflowing an eight bit counter and would need the first
   compare to fire very shortly after the start of the cycle.  Instead
   split the pulse into two overflows and run two at once to get a
   fast enough update rate.

   So:
    * Run two channels at once.
    * Use OCR0A for channel 0.
    * Use OCR0B for channel 1.
    * On overflow, schedule the rising edge.
    * On mid overflow, schedule the falling edge.
*/

const uint8_t Servos::pins_[] = {
    _BV(0), _BV(1), _BV(2), _BV(3), _BV(4), _BV(5),
};

Servos::Servos()
    : at_(0), state_(State::Start) {

    static_assert(NumChannels % 2 == 0, "Need an even number of channels.");

    for (volatile int8_t& position : position_) {
        position = Mid;
    }
}

void Servos::init() {
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

void Servos::set(uint8_t channel, int8_t position) {
    position_[channel] = position;
}

inline void Servos::overflow() {
    uint8_t at = at_;

    if (state_ == State::Start) {
        at += 2;

        if (at >= NumChannels) {
            at = 0;
            /* Be conservitative and reset the port each cycle. */
            PORTC = 0;
        }

        // TODO(michaelh): check that this is cheap.
        OCR0A = -(uint8_t)((position_[at+0] + Bias)/2);
        OCR0B = -(uint8_t)((position_[at+1] + Bias)/2);

        at_ = at;
        state_ = State::Centre;
    } else {
        OCR0A = (uint8_t)((position_[at+0] + Bias + 1)/2);
        OCR0B = (uint8_t)((position_[at+1] + Bias + 1)/2);

        state_ = State::Start;
    }
}

inline void Servos::compare_a() {
    PINC = pins_[at_+0];
}

inline void Servos::compare_b() {
    PINC = pins_[at_+1];
}

ISR(TIMER0_OVF_vect) {
    RoverIf::servos.overflow();
    HAL::ticks++;
}

ISR(TIMER0_COMPA_vect) {
    RoverIf::servos.compare_a();
}

ISR(TIMER0_COMPB_vect) {
    RoverIf::servos.compare_b();
}
