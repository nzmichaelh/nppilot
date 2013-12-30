#include "supervisor.h"
#include "roverif.h"

#include <algorithm>

/* Goals:

 * Throttle shutdown on start.
 * Throttle shutdown when returning to remote.
 * Clear throttle shutdown by going to idle.
 * None, Remote, or Pilot is in control.
 * Can enter pilot mode when enabled by a switch on the remote.
 * Enter pilot mode immediately without shutdowns.

 */

Supervisor::Supervisor()
    : in_control_(InControl::Invalid), in_shutdown_(true) {
}

void Supervisor::init() {
    change(InControl::Initial);
    changed();
}

void Supervisor::update_remote(bool throttle_high, bool pilot_allowed) {
    bool old = pilot_allowed_;

    throttle_high_ = throttle_high;
    pilot_allowed_ = pilot_allowed;

    remote_seen_.start(500);
    remote_ok_ = true;
    check();

    if (old != pilot_allowed_) {
        changed();
    }
}

void Supervisor::update_pilot(bool want_control) {
    pilot_wants_ = want_control;

    pilot_seen_.start(500);
    pilot_ok_ = true;
    check();
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
    if (remote_ok_ && pilot_ok_) {
        if (pilot_wants_ && pilot_allowed_ && !in_shutdown_) {
            change(InControl::Pilot);
        } else {
            change(InControl::Remote);
        }
    } else if (remote_ok_) {
        change(InControl::Remote);
    } else {
        change(InControl::None);
    }

    if (in_control_ == InControl::Remote) {
        if (in_shutdown_ && !throttle_high_) {
            in_shutdown_ = false;
            changed();
        }
    }
}

void Supervisor::change(InControl next) {
    if (in_control_ == InControl::Initial && next == InControl::None) {
        // Stay in initial.
    } else if (in_control_ != next) {
        if (next != InControl::Pilot) {
            if (!in_shutdown_) {
                shutdown();
            }
            in_shutdown_ = true;
        }
        in_control_ = next;
        changed();
    }
}
