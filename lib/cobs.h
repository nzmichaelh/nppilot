#pragma once

#include <cstdint>

class COBSSink
{
public:
    void send(int id, const void* msg, int length);

private:
    static const uint8_t Frame = '~';
    static const uint8_t NoZero = 128;
    static const uint8_t Longest = 120;

    void feed(const void* buffer, int length);
    void segment(const uint8_t* start, const uint8_t* end, bool no_zero);
    void write(const uint8_t* p, int length);
};
