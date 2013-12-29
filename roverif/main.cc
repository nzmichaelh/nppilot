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
MinBitArray RoverIf::pending;

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

void RoverIf::fill_state(Protocol::State* pmsg) {
    *pmsg = {
        .code = Protocol::Code::State,
        .flags = Protocol::State::Flags::None,
    };
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

void Supervisor::changed() {
    static const uint8_t patterns[][2] = {
        [State::None] =        { 0b101,      0 },
        [State::Initial] =     { 0b10000001, 0 },
        [State::Remote] =      { 0,          0b10000001 },
        [State::RemoteArmed] = { 0,          0b10000101 },
        [State::Pilot] =       { 0,          0b11111110 },
        [State::Shutdown] =    { 0b1011,     0 },
    };

    int idx = static_cast<int>(state());
    RoverIf::blinker.set(patterns[idx][0], patterns[idx][1]);
}

inline bool RoverIf::tick_one(Timer& timer, int divisor) {
    return timer.tick(Timer::round(HAL::TicksPerSecond, divisor));
}

void RoverIf::tick() {
    if (tick_one(pwmin_timer, 10)) {
        defer(Pending::PWMIn);
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
    supervisor.set_pilot(msg.flags == Protocol::Demand::Flags::TakeControl, nullptr, 0);
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

    if (!pending.is_empty()) {
        void* pmsg = link.start();

        if (pmsg != nullptr) {
            Pending next = (Pending)pending.pop();
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
