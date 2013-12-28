#include "supervisor.h"
#include "roverif.h"

#include <algorithm>

const Switch::Fixed Supervisor::mode_fixed_ = {
    { 0, 6310, 9160, 12000 }
};


Supervisor::Supervisor()
    : state_(State::None), remote_ok_(false), pilot_ok_(false) {
}

void Supervisor::init() {
    change(State::Initial);
}

void Supervisor::set_remote(const uint16_t* channels, int count) {
    if (std::abs(channels[ThrottleChannel] - LostThrottle) > 10) {
        remote_seen_.start(500);
        remote_ok_ = true;
    }

    mode_.update(mode_fixed_, channels[4]);
    pilot_allowed_ = (mode_.position() == 3);

    check();
}

void Supervisor::set_pilot(
    bool in_control, const uint16_t* channels, int count) {
}

void Supervisor::tick() {
    remote_seen_.tick();
    pilot_seen_.tick();

    if (!remote_seen_.running()) {
        remote_ok_ = false;
        check();
    }

    if (!pilot_seen_.running()) {
        pilot_ok_ = false;
        check();
    }
}

void Supervisor::check() {
    if (remote_ok_ && pilot_ok_ && pilot_allowed_ && pilot_wants_) {
        change(State::Pilot);
    } else if (remote_ok_ && pilot_allowed_) {
        change(State::RemoteArmed);
    } else if (remote_ok_) {
        change(State::Remote);
    } else {
        if (state_ != State::Initial) {
            change(State::Shutdown);
        }
    }
}

void Supervisor::change(State next) {
    if (state_ != next) {
        state_ = next;
        changed();

        if (state_ == State::Shutdown) {
            /* PENDING: Use NUM_ELEMS */
            shutdown();
        }
    }
}

void Supervisor::shutdown() {
}

void Supervisor::update(const uint16_t* channels, int count) {
}
