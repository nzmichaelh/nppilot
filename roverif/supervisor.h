/*
 * Supervises access to the control surfaces.
 */

#pragma once

#include <stdint.h>
#include <timer.h>
#include <switch.h>

class Supervisor
{
public:
    enum class State {
        None,
        Remote,
        RemoteArmed,
        Pilot,
        Shutdown,
    };

    Supervisor();

    void set_remote(const uint16_t* channels, int count);
    void set_pilot(bool in_control, const uint16_t* channels, int count);

    State state() const { return state_; }

    void expired();

    static Timer remote_seen_;
    static Timer::Fixed remote_seen__fixed;
    static Timer pilot_seen_;
    static Timer::Fixed pilot_seen__fixed;

private:
    static const int LostThrottle = 8296;
    static const int ThrottleChannel = 2;
    static const int ModeChannel = 2;

    void update(const uint16_t* channels, int count);
    void shutdown();
    void changed();

    void check();
    void change(State next);

    State state_;

    bool remote_ok_;
    bool pilot_ok_;
    bool pilot_allowed_;
    bool pilot_wants_;
    Switch mode_;

    static const Switch::Fixed mode_fixed_;
};
