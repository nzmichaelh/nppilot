#include "cobs.h"

struct Header {
    uint8_t id;
    uint8_t sum;
};

void COBSSink::feed(const void* abuffer, int length)
{
    const uint8_t* buffer = (const uint8_t*)abuffer;
    const uint8_t* start = buffer;
    const uint8_t* p;

    for (p = buffer; p != buffer + length; p++) {
        uint8_t ch = *p;

        if (ch == Frame) {
            segment(start, p, false);
            start = p + 1;
        }
        else {
            int seen = p - start;

            if (seen >= Longest) {
                segment(start, p, true);
                start = p;
            }
        }
    }

    if (start != p) {
        segment(start, p, true);
    }
}

void COBSSink::segment(const uint8_t* start, const uint8_t* end, bool no_zero)
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

void COBSSink::send(int id, const void* amsg, int length)
{
    const uint8_t* msg = (const uint8_t*)amsg;

    int sum = id;

    for (int i = 0; i < length; i++) {
        sum += msg[i];
    }

    Header header = { .id = (uint8_t)id, .sum = (uint8_t)~sum  };

    feed(&header, sizeof(header));
    feed(msg, length);
    
    uint8_t ch = Frame;
    write(&ch, 1);
}
