/*
 * Link across the USB CDCACM connection.
 */

#pragma once

#include <ringbuffer.h>

/**
 * Link across the USB CDCACM connection.
 * Buffers up writes which are flushed on full or a call to flush().
 */
class USBLink
{
public:
    USBLink();

    void write(const uint8_t* data, int length);
    bool flush();

private:
    RingBuffer<uint8_t, 128> tx_;
};
