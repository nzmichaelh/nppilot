/*
 * Supervises access to the control surfaces.
 */

#pragma once

#include <stdint.h>
#include <timer.h>
#include <switch.h>

class Supervisor {
 public:
    enum class InControl {
        Invalid,
        Initial,
        None,
        Remote,
        Pilot,
    };

    Supervisor();

    void init();

    void update_remote(bool throttle_high, bool pilot_allowed);
    void update_pilot(bool want_control);

    bool pilot_allowed() const { return pilot_allowed_; }
    bool in_shutdown() const { return in_shutdown_; }
    bool remote_ok() const { return remote_ok_; }
    InControl in_control() const { return in_control_; }

    void tick();

 private:
    void check();
    void change(InControl next);
    void shutdown();
    void changed();

    InControl in_control_;

    bool remote_ok_;
    bool pilot_ok_;
    bool pilot_allowed_;
    bool pilot_wants_;
    bool throttle_high_;
    bool in_shutdown_;

    Timer remote_seen_;
    Timer pilot_seen_;
};
