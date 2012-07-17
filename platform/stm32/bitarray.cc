#include <bitarray.h>

#include <libmaple/bitband.h>

uint32_t* BitArray::get_base() const
{
    uint32_t p = (uint32_t)&_bits;
    uint32_t* pbase = (uint32_t*)BB_SRAM_BASE;

    return pbase + 8 * (p - BB_SRAM_REF);
}

void BitArray::set(int index, bool value)
{
    check(index);

    get_base()[index] = value;
}

int BitArray::last_set() const
{
    /* ARM clz is defined when v==0 */
    return count() - 1 - __builtin_clz(_bits);
}
