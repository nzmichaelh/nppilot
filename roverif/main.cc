#include <cstring>
#include <cstdint>
#include <cstdio>

#include "roverif.h"
#include "hal.h"

PWMIn RoverIf::pwmin;
Switcher RoverIf::switcher;
USBLink RoverIf::usblink;
COBSLink RoverIf::link;
Blinker RoverIf::blinker;
Supervisor RoverIf::supervisor;

void COBSLink::write(const uint8_t* p, int length)
{
    RoverIf::usblink.write(p, length);
}

void Timer::dispatch(int id)
{
    RoverIf::switcher.trigger(id);
}

void COBSLink::dispatch(int id, const void* msg, int length)
{
    Debug::info("COBSLink::dispatch");
}

static void heartbeat()
{
    Protocol::Heartbeat msg = { .version = 1, .device_id = 2 };
    RoverIf::link.send('h', &msg, sizeof(msg));
}

void Blinker::update(bool level)
{
    HAL::set_status_led(level);
}

void Supervisor::changed()
{
    static const uint16_t patterns[] = {
        [State::None] =        0b101,
        [State::Remote] =      0b100000101,
        [State::RemoteArmed] = 0b100001101,
        [State::Pilot] =       0b111111110,
        [State::Shutdown] =    0b100000001,
    };

    RoverIf::blinker.set(patterns[(int)state()]);
}

static void tick()
{
    Timer::tick_all();
}

MAKE_TIMER(timer_expired, -1, Timer::Stopped);
MAKE_TIMER(blinker_tick, BlinkerID, 100);
 
void RoverIf::poll()
{
    Protocol::Inputs msg = { };

    for (int i = 0; i < pwmin.count(); i++) {
        msg.channel[i] = pwmin.value(i);
    }

    supervisor.set_remote(msg.channel, 6);
    link.send('i', &msg, sizeof(msg));
}

MAKE_TIMER(timer_heartbeat, HeartbeatID, 1000);
MAKE_TIMER(timer_poll, PollID, 20);

void Switcher::dispatch(int id)
{
    switch (id) {
    case BlinkerID:
        RoverIf::blinker.tick();
        break;
    case SysTickID:
        tick();
        break;
    case HeartbeatID:
        heartbeat();
        break;
    case PollID:
        RoverIf::poll();
        break;
    case SupervisorID:
        RoverIf::supervisor.expired();
        break;
    default:
        assert(false);
        break;
    }
}

void RoverIf::init()
{
    HAL::init();
}

void RoverIf::run()
{
    HAL::start();
    Debug::info("roverif: start");

    blinker.set(0b101);

    for (;;) {
        switcher.next();
        usblink.flush();
        HAL::wait();
    }
}
