/*
 * Array of bits.
 */

#pragma once

#include <cstdint>
#include <cassert>

/**
 * Array of bits used for scheduling work.
 */
class MinBitArray {
 public:
    MinBitArray() : bits_(0) {}

    /** Sets an individual bit. */
    void set(int index) { check(index); bits_ |= 1 << index; }

    bool is_empty() const { return bits_ == 0; }
    /** Finds the lowest set bit, clears it, and returns the index. */
    int8_t pop();

 private:
    int count() const { return sizeof(bits_)*8; }

    void check(int index) {
        assert(index >= 0 && index < count());
    }

    uint8_t bits_;
};
