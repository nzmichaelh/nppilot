#include "blinker.h"

Blinker::Blinker()
    : pattern_(0), reload_(0), at_(0)
{
}

void Blinker::tick()
{
    set((pattern_ & (1 << at_)) != 0);

    if (++at_ == 8*sizeof(pattern_)) {
        pattern_ = reload_;
        at_ = 0;
    }
}
