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

Timer blinker_timer;
Timer heartbeat_timer;

Link::Link()
    : tx_at_(0), tx_end_(0)
{
}

void* Link::start()
{
    if (tx_at_ >= tx_end_) {
        return tx_;
    } else {
        return nullptr;
    }
}

void Link::send(uint8_t length)
{
    uint8_t* p;
    uint16_t sum1 = 0x12;
    uint16_t sum2 = 0x34;
    for (p = tx_; p < tx_ + length; p++) {
        sum1 += *p;
        if (sum1 >= 255) {
            sum1 -= 255;
        }
        sum2 += sum1;
        if (sum2 >= 255) {
            sum2 -= 255;
        }
    }
    *p = (uint8_t)(sum1 ^ sum2);

    tx_end_ = length + 1;
    tx_at_ = 0;
}

void RoverIf::heartbeat()
{
    Protocol::Heartbeat* pmsg = (Protocol::Heartbeat*)link.start();

    if (pmsg != nullptr) {
        *pmsg = {
            .code = 'h',
            .version = 1,
            .device_id = 2,
        };
        link.send(sizeof(*pmsg));
    }
}

void Supervisor::changed()
{
    static const uint8_t patterns[][2] = {
        [State::None] =        { 0, 0b10000001 },
        [State::Remote] =      { 0b10000001, 0 },
        [State::RemoteArmed] = { 0b10000101, 0 },
        [State::Pilot] =       { 0b11111110, 0 },
        [State::Shutdown] =    { 0, 0b1011 },
    };

    int idx = (int)state();
    RoverIf::blinker.set(patterns[idx][0], patterns[idx][1]);
}

void RoverIf::tick()
{
    if (blinker_timer.tick(HAL::TicksPerSecond / 8)) {
        RoverIf::blinker.tick();
    }
    if (heartbeat_timer.tick(HAL::TicksPerSecond / 10)) {
        heartbeat();
    }
    RoverIf::supervisor.tick();
}

void RoverIf::poll()
{
}

void RoverIf::init()
{
    HAL::init();
    Servos::init();
}

void RoverIf::run()
{
    HAL::start();
    blinker.set(0, 0b101);

    for (;;) {
        HAL::poll();
        HAL::wait();

        if (ticks_ != HAL::ticks) {
            ticks_++;
            tick();
        }
    }
}

void run()
{
    RoverIf::init();
    RoverIf::run();
}
