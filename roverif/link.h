#pragma once

#include <cstdint>

class Link
{
public:
    Link();

    /** Send a frame */
    void* start();
    void send(uint8_t length);

    /**
     * Feed received data, dispatching received messages as they're
     * decoded.
     */
    void feed(const uint8_t* data, int length);

    void tx_next();

private:
    static const uint8_t Mark = '\n';
    static const uint8_t Escape = ' ';

    void putch(uint8_t ch);

    uint8_t tx_[8];
    uint8_t tx_at_;
    uint8_t tx_end_;
};
