#include "cobs.h"

COBSLink::COBSLink()
    : at_(0), remain_(0)
{
}

void COBSLink::send_part(const void* abuffer, int length)
{
    const uint8_t* buffer = (const uint8_t*)abuffer;
    const uint8_t* start = buffer;
    const uint8_t* p;

    for (p = buffer; p != buffer + length; p++) {
        uint8_t ch = *p;

        if (ch == Frame) {
            send_segment(start, p, false);
            start = p + 1;
        }
        else {
            int seen = p - start;

            if (seen >= Longest) {
                send_segment(start, p, true);
                start = p;
            }
        }
    }

    if (start != p) {
        send_segment(start, p, true);
    }
}

void COBSLink::send_segment(const uint8_t* start, const uint8_t* end, bool no_zero)
{
    int seen = end - start;

    if (no_zero) {
        seen |= NoZero;
    }
    else {
        seen += 1;
    }

    uint8_t ch = seen + Frame;
    write(&ch, 1);
    write(start, end - start);
}

void COBSLink::send(int id, const void* amsg, int length)
{
    const uint8_t* msg = (const uint8_t*)amsg;

    int sum = id;

    for (int i = 0; i < length; i++) {
        sum += msg[i];
    }

    static_assert(sizeof(Header) == 2, "COBSLink::Header must be two bytes.");
    Header header = { .id = (uint8_t)id, .sum = (uint8_t)~sum  };

    send_part(&header, sizeof(header));
    send_part(msg, length);
    
    uint8_t ch = Frame;
    write(&ch, 1);
}

uint8_t COBSLink::sum()
{
    uint8_t total = 0;

    for (int i = 0; i < at_; i++) {
        total += rx_[i];
    }

    return total;
}

void COBSLink::append(uint8_t ch)
{
    if (at_ >= sizeof(rx_)) {
        at_ = sizeof(rx_) + 1;
    } else {
        rx_[at_++] = ch;
    }
}

void COBSLink::feed(const uint8_t* data, int length)
{
    /* Rely on load hoisting to optimise at_ */

    for (int i = 0; i < length; i++) {
        uint8_t ch = *data++;

        if (ch == Frame) {
            if (at_ < sizeof(Header)) {
                /* Too short */
            } else if (at_ > sizeof(rx_)) {
                /* Too long */
            } else if (sum() != 0xFF) {
                /* Bad checksum */
            } else {
                Header* pheader = (Header*)rx_;
                dispatch(pheader->id, pheader->body, at_ - sizeof(*pheader));
            }

            /* Reset */
            remain_ = 0;
            at_ = 0;
        } else {
            /* Non framing character */
            if (remain_ == 0) {
                remain_ = ch - Frame;
                zero_ = !(remain_ & NoZero);
                remain_ &= ~NoZero;

                if (zero_) {
                    remain_--;
                }
            } else {
                append(ch);
                remain_--;
            }

            if (remain_ == 0 && zero_) {
                append(Frame);
            }
        }
    }
}
