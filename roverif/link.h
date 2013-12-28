#pragma once

#include <cstdint>

class Link {
 public:
    Link();

    /** Send a frame */
    void* start();
    void send(uint8_t length);

    const void* peek(uint8_t* plength);
    void discard() { rx_at_ = 0; rx_full_ = false; }

    void tx_next();
    void rx_next();

 private:
    static const uint8_t Mark = '\n';
    static const uint8_t Escape = '^';
    static const uint8_t Xor = 0x20;

    void putch(uint8_t ch);
    uint8_t checksum(const uint8_t* p, uint8_t length);

    uint8_t tx_[8];
    uint8_t tx_at_;
    uint8_t tx_end_;

    uint8_t rx_[8];
    uint8_t rx_at_;
    uint8_t rx_xor_;
    bool rx_full_;
};
