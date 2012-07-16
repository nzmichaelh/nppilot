/*
 * Array of bits.
 */

#include <cstdint>
#include <cassert>

/**
 * Array of bits.  Atomic where possible.
 */
class BitArray
{
public:
    BitArray() : _bits(0) {}

    /** Sets or clears an individual bit. */
    void set(int index, bool value);

    /** Get the number of elements. */
    int count() const { return 8*sizeof(_bits); }

    /** Get the value of a bit. */
    bool item(int index) const;

    /** Get the index of the last set bit or -1 if none. */
    int last_set() const;

private:
    void check(int index) const { assert(index >= 0 && index < count()); }
    uint32_t* get_base() const;

    uint32_t _bits;
};

inline bool BitArray::item(int index) const
{
    check(index);
    return (_bits & (1 << index)) != 0;
}
