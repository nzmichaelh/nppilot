#include "link.h"
#include "roverif.h"
#include <cassert>
#include <avr/io.h>
#include <avr/interrupt.h>

Link::Link()
    : tx_at_(1), tx_end_(0)
{
}

inline void Link::putch(uint8_t ch)
{
    UDR0 = ch;
}

void* Link::start()
{
    if (tx_at_ > tx_end_) {
        return tx_;
    } else {
        return nullptr;
    }
}

void Link::send(uint8_t length)
{
    assert(length > 0);
    assert(tx_at_ > tx_end_);

    uint8_t* p;
    uint16_t sum1 = 0x12;
    uint16_t sum2 = 0x34;
    for (p = tx_; p < tx_ + length; p++) {
        sum1 += *p;
        if (sum1 >= 255) {
            sum1 -= 255;
        }
        sum2 += sum1;
        if (sum2 >= 255) {
            sum2 -= 255;
        }
    }
    *p = (uint8_t)(sum1 ^ sum2);

    tx_end_ = length + 1;
    tx_at_ = 1;

    putch(tx_[0]);
}

inline void Link::tx_next()
{
    if (tx_at_ > tx_end_) {
        // Nothing to do.
    } else if (tx_at_ == tx_end_) {
        putch(Mark);
        tx_at_++;
    } else {
        uint8_t next = tx_[tx_at_];

        if (next == Mark || next == Escape) {
            putch(Escape);
            tx_[tx_at_] = next ^ Escape;
        } else {
            putch(next);
            tx_at_++;
        }
    }
}

ISR(USART_TX_vect)
{
    RoverIf::link.tx_next();
}
