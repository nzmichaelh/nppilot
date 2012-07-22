/*
 * A variant of Constant Overhead Byte Stuffing for sending frames
 * over a serial link.
 */

#pragma once

#include <cstdint>

/**
 * A variant of Constant Overhead Byte Stuffing for sending frames
 * over a serial link.
 *
 * A frame is id, checksum, and body and is terminated by the end of
 * frame marker.  The encoding is roughly readable over a terminal by
 * using an ASCII printable for the ID and newline for frame marker.
 * The checksum is the 1-complements of the sum of the ID and body.
 *
 * COBS ensures the frame marker doesn't appear in the body by writing
 * fragments that start with a count and implicitly include the frame
 * marker at the end.
 */
class COBSLink
{
public:
    COBSLink();

    /** Send a frame */
    void send(int id, const void* msg, int length);

    /**
     * Feed received data, dispatching received messages as they're
     * decoded.
     */
    void feed(const uint8_t* data, int length);

private:
    struct Header {
        uint8_t id;
        uint8_t sum;
        uint8_t body[0];
    };

    static const uint8_t Frame = '\n';
    static const uint8_t NoZero = 128;
    static const uint8_t Longest = 120;

    static const int MaxFrame = 32;

    /**
     * Called when a good frame has been received.  msg may be
     * unaligned.
     */
    void dispatch(int id, const void* msg, int length);

    void send_part(const void* buffer, int length);
    void send_segment(const uint8_t* start, const uint8_t* end, bool no_zero);
    void write(const uint8_t* p, int length);

    uint8_t sum();
    void append(uint8_t ch);

    uint8_t rx_[MaxFrame];
    uint8_t at_;
    uint8_t remain_;
    bool zero_;
};
