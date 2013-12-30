#include "pwmin.h"
#include "roverif.h"

#include <avr/io.h>
#include <avr/interrupt.h>

// All are on PORTB

PWMIn::PWMIn() {
    for (Input& input : inputs_) {
        input.good = 0;
    }
}

void PWMIn::init() {
    DDRB = 0;
    PORTB = 0xFF;
    PCMSK0 = 0xFF;
    PCICR = _BV(PCIE0);
}

void PWMIn::expire() {
    for (Input& input : inputs_) {
        switch (input.good) {
        case 0:
            break;
        case 1:
            input.width = 0;
            input.good = 0;
            break;
        default:
            input.good--;
            break;
        }
    }
}

int8_t PWMIn::get(uint8_t channel) const {
    const Input& input = inputs_[channel];

    if (input.good >= Saturate/2) {
        return input.width;
    } else {
        return Missing;
    }
}

inline void PWMIn::pcint() {
    uint8_t now = TCNT0;
    uint8_t level = PINB;
    uint8_t changed = (level_ ^ level) & ((1 << NumChannels)-1);
    level_ = level;

    for (uint8_t i = 0;
         changed != 0;
         changed >>= 1, level >>= 1, i++) {
        if ((changed & 1) != 0) {
            Input& input = inputs_[i];
            if ((level & 1) != 0) {
                // Rising edge.
                input.rose_at = now;
            } else {
                // Falling edge.
                input.width =
                    now - input.rose_at - HAL::Timer0PerMillisecond*Center/100;
                if (input.good < Saturate) {
                    input.good++;
                }
                if (i == 0) {
                    // ~62 cycles per second.
                    cycles++;
                }
            }
        }
    }
}

ISR(PCINT0_vect) {
    RoverIf::pwmin.pcint();
}
