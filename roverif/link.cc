#include "link.h"
#include "roverif.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include <cassert>

Link::Link()
    : tx_at_(1), tx_end_(0),
      rx_at_(0), rx_xor_(0) {
}

uint8_t Link::checksum(const uint8_t* p, uint8_t length) {
    uint16_t sum1 = 0x12;
    uint16_t sum2 = 0x34;

    for (; length != 0; length--, p++) {
        sum1 += *p;
        if (sum1 >= 255) {
            sum1 -= 255;
        }
        sum2 += sum1;
        if (sum2 >= 255) {
            sum2 -= 255;
        }
    }

    return (uint8_t)(sum1 ^ sum2);
}

inline void Link::disable_tx() {
    UCSR0B &= ~_BV(UDRIE0);
}

inline void Link::enable_tx() {
    UCSR0B |= _BV(UDRIE0);
}

inline void Link::putch(uint8_t ch) {
    UDR0 = ch;
}

void* Link::start() {
    if (tx_at_ > tx_end_) {
        return tx_;
    } else {
        return nullptr;
    }
}

void Link::send(uint8_t length) {
    assert(length > 0);
    assert(tx_at_ > tx_end_);

    tx_[length] = checksum(tx_, length);

    tx_end_ = length + 1;
    tx_at_ = 0;

    enable_tx();

    sent++;
}

const void* Link::peek(uint8_t* plength) {
    if (!rx_full_) {
        return nullptr;
    } else if (rx_at_ < 2) {
        // Need at least the code and checksum.
        discard();
        rx_errors++;
        return nullptr;
    } else {
        uint8_t len = rx_at_ - 1;
        uint8_t sum = checksum(rx_, len);

        if (sum != rx_[len]) {
            discard();
            rx_errors++;
            return nullptr;
        } else {
            *plength = len;
            received++;
            return rx_;
        }
    }
}

inline void Link::tx_next() {
    if (tx_at_ > tx_end_) {
        // Nothing to do.
    } else if (tx_at_ == tx_end_) {
        putch(Mark);
        tx_at_++;
        disable_tx();
    } else {
        uint8_t next = tx_[tx_at_];

        if (next == Mark || next == Escape) {
            putch(Escape);
            tx_[tx_at_] = next ^ Xor;
        } else {
            putch(next);
            tx_at_++;
        }
    }
}

inline void Link::rx_next() {
    uint8_t ch = UDR0;

    if (!rx_full_) {
        if (ch == Mark) {
            rx_full_ = true;
        } else if (ch == Escape) {
            rx_xor_ = Xor;
        } else {
            if (rx_at_ == sizeof(rx_)) {
                // Overflow
            } else {
                rx_[rx_at_++] = ch ^ rx_xor_;
                rx_xor_ = 0;
            }
        }
    }
}

ISR(USART_RX_vect) {
    RoverIf::link.rx_next();
}

ISR(USART_UDRE_vect) {
    RoverIf::link.tx_next();
}
