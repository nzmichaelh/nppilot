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

    static const int RxBufferSize = 12;
    static const int TxBufferSize = 22;

    void putch(uint8_t ch);
    void disable_tx();
    void enable_tx();

    uint8_t checksum(const uint8_t* p, uint8_t length);

    uint8_t tx_[TxBufferSize];
    uint8_t tx_at_;
    uint8_t tx_end_;

    uint8_t rx_[RxBufferSize];
    uint8_t rx_at_;
    uint8_t rx_xor_;
    bool rx_full_;
};
