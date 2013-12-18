#include <cstring>
#include <cstdint>
#include <cstdio>

#include "roverif.h"
#include "hal.h"

PWMIn RoverIf::pwmin;
Switcher RoverIf::switcher;
Link RoverIf::link;
Blinker RoverIf::blinker;
Supervisor RoverIf::supervisor;
uint8_t RoverIf::ticks_;
MinBitArray RoverIf::pending;

Timer blinker_timer;
Timer heartbeat_timer;
Timer pwmin_timer;

void RoverIf::send_heartbeat(Protocol::Heartbeat& msg)
{
    msg = {
        .code = Protocol::Code::Heartbeat,
        .version = 1,
        .device_id = 2,
        .ticks = HAL::ticks,
        .state = (uint8_t)supervisor.state(),
    };
    link.send(sizeof(msg));
}

void RoverIf::send_pwmin(Protocol::Inputs& msg)
{
    msg.code = Protocol::Code::Inputs;

    for (uint8_t i = 0; i < sizeof(msg.channels); i++) {
        msg.channels[i] = pwmin.get(i);
    }
    link.send(sizeof(msg));
}

void RoverIf::send_pong(Protocol::Pong& msg)
{
    msg = {
        .code = Protocol::Code::Pong,
    };
    link.send(sizeof(msg));
}

void Supervisor::changed()
{
    static const uint8_t patterns[][2] = {
        [State::None] =        { 0b10000001, 0 },
        [State::Remote] =      { 0, 0b10000001 },
        [State::RemoteArmed] = { 0, 0b10000101 },
        [State::Pilot] =       { 0, 0b11111110 },
        [State::Shutdown] =    { 0b1011, 0 },
    };

    int idx = (int)state();
    RoverIf::blinker.set(patterns[idx][0], patterns[idx][1]);
}

void RoverIf::tick()
{
    if (pwmin_timer.tick(HAL::TicksPerSecond / 10)) {
        defer(Pending::PWMIn);
    }
    if (blinker_timer.tick(HAL::TicksPerSecond / 7)) {
        RoverIf::blinker.tick();
    }
    if (heartbeat_timer.tick(HAL::TicksPerSecond / 2)) {
        defer(Pending::Heartbeat);
    }
    RoverIf::supervisor.tick();
}

void RoverIf::handle_ping(const Protocol::Ping& msg)
{
    defer(Pending::Pong);
}

#define DISPATCH(_type, _handler) \
    case Protocol::Code::_type: \
    if (length == sizeof(Protocol::_type)) \
        _handler(*(const Protocol::_type*)p); \
    break

#define DISPATCH_PENDING(_code, _type, _handler) \
    case Pending::_code:                         \
    _handler (*(Protocol::_type*)pmsg); \
    break

void RoverIf::poll()
{
    uint8_t length;
    const void* p = link.peek(length);
    
    if (p != nullptr) {
        switch (*(const Protocol::Code*)p) {
            DISPATCH(Ping, handle_ping);
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
                DISPATCH_PENDING(PWMIn, Inputs, send_pwmin);
                DISPATCH_PENDING(Heartbeat, Heartbeat, send_heartbeat);
                DISPATCH_PENDING(Pong, Pong, send_pong);
            }
        }
    }
}

void RoverIf::init()
{
    HAL::init();
    Servos::init();
    pwmin.init();
}

void RoverIf::run()
{
    HAL::start();
    blinker.set(0, 0b101);

    for (;;) {
        HAL::poll();
        poll();
        HAL::wait();
    }
}

void run()
{
    RoverIf::init();
    RoverIf::run();
}
