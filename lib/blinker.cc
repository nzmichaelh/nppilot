#include "blinker.h"

Blinker::Blinker()
    : red_(0), red_reload_(0),
      green_(0), green_reload_(0) {
}

void Blinker::tick() {
    uint8_t combined = red_ | green_;

    if (combined <= 1) {
        red_ = red_reload_;
        green_ = green_reload_;
    }

    update((red_ & 1) != 0, (green_ & 1) != 0);
    red_ >>= 1;
    green_ >>= 1;
}
