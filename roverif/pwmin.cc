#include <string.h>

#include "pwmin.h"

PWMIn::PWMIn()
    : recent_(0)
{
    memset(value_, 0, sizeof(value_));
    memset(last_, 0, sizeof(last_));
}

void PWMIn::irq(int channel, uint32_t ccr, volatile uint32_t* pccer, int mask)
{
    uint32_t ccer = *pccer;
    *pccer = ccer ^ mask;

    if ((ccer & mask) == 0) {
        /*
         * Use zero as the special 'never seen' value.  Slight chance
         * this happens in practice but rare and rapidly overwritten.
         */
        if (last_[channel] != 0) {
            value_[channel] = ccr - last_[channel];
            recent_ |= 1 << channel;
        }
    }

    last_[channel] = ccr;
}

void PWMIn::expire()
{
    int recent = recent_;
    /*
     * Mild race here.  The IRQ could write back the old value and
     * imply that other channels have been seen.  That'll be fixed up
     * next round.  XOR might work better.
     */
    recent_ = 0;

    for (int i = 0; i < Count; i++) {
        if ((recent & (1 << i)) == 0) {
            value_[i] = Missing;
            last_[i] = 0;
            /* PENDING: doesn't handle the initial glitch in last_ */
        }
    }
}

