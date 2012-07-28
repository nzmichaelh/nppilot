#include "blinker.h"

Blinker::Blinker()
    : pattern_(0), reload_(0)
{
}

void Blinker::tick()
{
    if (pattern_ <= 1) {
        pattern_ = reload_;
    }

    update((pattern_ & 1) != 0);
    pattern_ >>= 1;
}
