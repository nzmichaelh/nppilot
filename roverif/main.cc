#include <cstring>
#include <cstdint>
#include <cstdio>

#include "roverif.h"
#include "hal.h"
#include "version.h"

PWMIn RoverIf::pwmin;
Servos RoverIf::servos;
Link RoverIf::link;
Blinker RoverIf::blinker;
Supervisor RoverIf::supervisor;
uint8_t RoverIf::ticks_;
MinBitArray RoverIf::pending_;

Timer blinker_timer;
Timer heartbeat_timer;
Timer pwmin_timer;
Timer state_timer;

void RoverIf::fill_heartbeat(Protocol::Heartbeat* pmsg) {
    *pmsg = {
        .code = Protocol::Code::Heartbeat,
        .version = 1,
        .device_id = 2,
        .ticks = HAL::ticks,
    };
}

void RoverIf::fill_pwmin(Protocol::Input* pmsg) {
    pmsg->code = Protocol::Code::Inputs;

    for (uint8_t i = 0; i < sizeof(pmsg->channels); i++) {
        pmsg->channels[i] = pwmin.get(i);
    }

    static uint8_t offset = Servos::Low;

    servos.set(0, offset);
    if (++offset > Servos::High)
        offset = Servos::Low;
}

static inline void set_flag(Protocol::State::Flags* pnow, Protocol::State::Flags next) {
    *pnow = (Protocol::State::Flags)((int)(*pnow) | (int)next);
}

void RoverIf::fill_state(Protocol::State* pmsg) {
    *pmsg = {
        .code = Protocol::Code::State,
        .flags = Protocol::State::Flags::None,
    };

    if (supervisor.remote_ok()) {
        set_flag(&pmsg->flags, Protocol::State::Flags::RemoteOK);
    }
    if (supervisor.in_shutdown()) {
        set_flag(&pmsg->flags, Protocol::State::Flags::InShutdown);
    }
    if (supervisor.in_control() == Supervisor::InControl::Pilot) {
        set_flag(&pmsg->flags, Protocol::State::Flags::InControl);
    }
    if (supervisor.pilot_allowed()) {
        set_flag(&pmsg->flags, Protocol::State::Flags::PilotAllowed);
    }
}

void RoverIf::fill_pong(Protocol::Pong* pmsg) {
    *pmsg = {
        .code = Protocol::Code::Pong,
    };
}

void RoverIf::fill_version(Protocol::Version* pmsg) {
    pmsg->code = Protocol::Code::Version;

    uint8_t at;
    for (at = 0; at < sizeof(pmsg->version) && version[at]; at++) {
        pmsg->version[at] = version[at];
    }

    for (; at < sizeof(pmsg->version); at++) {
        pmsg->version[at] = '\0';
    }
}

void RoverIf::poll_pwmin()
{
    if (pwmin.get(ShutdownChannel) < -30) {
        // Treat as missing.
    } else {
        supervisor.update_remote(
            abs(pwmin.get(ThrottleChannel)) > 10,
            pwmin.get(SwitchChannel) > -30
            );
    }
}

void RoverIf::send_state() {
    defer(Pending::State);
}

void Supervisor::changed() {
    uint8_t red = 0;
    uint8_t green = 0;

    switch (in_control()) {
    case InControl::Initial:
        red = 0b10000001;
        break;
    case InControl::None:
        red = 0b11111110;
        break;
    case InControl::Remote:
        green = pilot_allowed() ? 0b10001010 : 0b10000010;
        break;
    case InControl::Pilot:
        green = 0b11111110;
        break;
    case InControl::Invalid:
    default:
        red = 0b101;
        break;
    }

    if (in_shutdown() && red == 0) {
        red = 0b01;
    }
    RoverIf::blinker.set(red, green);
    RoverIf::send_state();
}

void Supervisor::shutdown() {
    RoverIf::servos.set(RoverIf::ThrottleChannel, Servos::Mid);
    RoverIf::send_state();
}

inline bool RoverIf::tick_one(Timer& timer, int divisor) {
    return timer.tick(Timer::round(HAL::TicksPerSecond, divisor));
}

void RoverIf::tick() {
    if (tick_one(pwmin_timer, 10)) {
        defer(Pending::PWMIn);
        poll_pwmin();
    }
    if (tick_one(blinker_timer, 7)) {
        RoverIf::blinker.tick();
    }
    if (tick_one(state_timer, 2)) {
        defer(Pending::State);
    }
    if (tick_one(heartbeat_timer, 2)) {
        defer(Pending::Heartbeat);
    }
    RoverIf::supervisor.tick();
}

#define MAP_REQUEST(_name) \
    case Protocol::Code::_name: defer(Pending::_name)

void RoverIf::handle_request(const Protocol::Request& msg) {
    switch (msg.requested) {
        MAP_REQUEST(Pong);
        MAP_REQUEST(Version);
    default:
        break;
    }
}

void RoverIf::handle_demand(const Protocol::Demand& msg) {
    supervisor.update_pilot(msg.flags == Protocol::Demand::Flags::TakeControl);
}

#define DISPATCH(_type, _handler) \
    case Protocol::Code::_type: \
    if (length == sizeof(Protocol::_type)) \
        _handler(*(const Protocol::_type*)p); \
    break

#define DISPATCH_PENDING(_code, _type, _handler) \
    case Pending::_code:                         \
    _handler((Protocol::_type*)pmsg); \
    link.send(sizeof(Protocol::_type)); \
    break

void RoverIf::poll() {
    uint8_t length;
    const void* p = link.peek(&length);

    if (p != nullptr) {
        switch (*(const Protocol::Code*)p) {
            DISPATCH(Request, handle_request);
            DISPATCH(Demand, handle_demand);
        default:
            break;
        }
        link.discard();
    }

    if (ticks_ != HAL::ticks) {
        ticks_++;
        tick();
    }

    if (!pending_.is_empty()) {
        void* pmsg = link.start();

        if (pmsg != nullptr) {
            Pending next = (Pending)pending_.pop();
            switch (next) {
                DISPATCH_PENDING(PWMIn, Input, fill_pwmin);
                DISPATCH_PENDING(State, State, fill_state);
                DISPATCH_PENDING(Heartbeat, Heartbeat, fill_heartbeat);
                DISPATCH_PENDING(Pong, Pong, fill_pong);
                DISPATCH_PENDING(Version, Version, fill_version);
            }
        }
    }
}

void RoverIf::init() {
    HAL::init();
    servos.init();
    pwmin.init();
    supervisor.init();
}

void RoverIf::run() {
    HAL::start();

    for (;;) {
        HAL::poll();
        poll();
        HAL::wait();
    }
}

void run() {
    RoverIf::init();
    RoverIf::run();
}
