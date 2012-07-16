#include <bitarray.h>

void BitArray::set(int index, bool value)
{
    check(index);

    if (value) {
        _bits |= 1 << index;
    } else {
        _bits &= ~(1 << index);
    }
}

int BitArray::last_set() const
{
    /* x86 clz isn't defined when v==0 */
    if (_bits == 0) {
        return -1;
    } else {
        return count() - 1 - __builtin_clz(_bits);
    }
}
