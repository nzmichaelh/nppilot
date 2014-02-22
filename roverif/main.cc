#include <cstring>
#include <cstdint>
#include <cstdio>

#include "roverif.h"
#include "hal.h"
#include "version.h"

Servos RoverIf::servos;
PWMIn RoverIf::pwmin;
IMU RoverIf::imu;
Link RoverIf::link;
Blinker RoverIf::blinker;
Supervisor RoverIf::supervisor;
uint8_t RoverIf::ticks_;
uint8_t RoverIf::pwmin_cycles_;
MinBitArray RoverIf::pending_;
uint8_t RoverIf::pilot_supplied_;
uint8_t RoverIf::demands_;

Timer imu_timer;
Timer check_timer;
Timer blinker_timer;
Timer heartbeat_timer;
Timer pwmin_limiter;
Timer state_timer;

void RoverIf::update_servos(const int8_t* pdemands, bool from_pilot) {
    Supervisor::InControl in_control = supervisor.in_control();
    bool use_pilot = (in_control == Supervisor::InControl::Pilot);

    if (from_pilot) {
        pilot_supplied_ = 0;

        if (use_pilot) {
            demands_++;
        }
    }

    for (int i = 0; i < servos.NumChannels; i++) {
        if (i == ThrottleChannel && supervisor.in_shutdown()) {
            servos.set(i, Servos::Mid);
        } else if (from_pilot) {
            if (pdemands[i] > Protocol::Demand::Reserved) {
                pilot_supplied_ |= 1 << i;

                if (use_pilot) {
                    servos.set(i, Servos::Mid + pdemands[i]);
                }
            }
        } else {
            if (pdemands[i] > PWMIn::Missing) {
                switch (in_control) {
                case Supervisor::InControl::Remote:
                    servos.set(i, Servos::Mid + pdemands[i]);
                    break;
                case Supervisor::InControl::Pilot:
                    if ((pilot_supplied_ & (1 << i)) == 0) {
                        servos.set(i, Servos::Mid + pdemands[i]);
                    }
                    break;
                default:
                    // No control.
                    break;
                }
            }
        }
    }
}

void RoverIf::fill_heartbeat(Protocol::Heartbeat* pmsg) {
    *pmsg = {
        .code = Protocol::Code::Heartbeat,
        .version = 1,
        .device_id = 2,
    };
}

void RoverIf::fill_pwmin(Protocol::Input* pmsg) {
    pmsg->code = Protocol::Code::Inputs;
    pmsg->reference = Protocol::Input::Reference12MHz;

    for (uint8_t i = 0; i < sizeof(pmsg->channels); i++) {
        pmsg->channels[i] = pwmin.get(i);
    }
}

bool RoverIf::fill_imu(Protocol::IMU* pmsg) {
    pmsg->code = Protocol::Code::IMU;
    return imu.read(pmsg);
}

template<typename T>
static inline void set_flag(T* pnow, T next) {
    *pnow = (T)(static_cast<int>(*pnow) | static_cast<int>(next));
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

void RoverIf::fill_counters(Protocol::Counters* pmsg) {
    *pmsg = {
        .code = Protocol::Code::Counters,
        .demands = demands_,
        .sent = link.sent,
        .received = link.received,
        .rx_errors = link.rx_errors,
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

void RoverIf::supervisor_changed() {
    pilot_supplied_ = 0;
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
    RoverIf::supervisor_changed();
}

void Supervisor::shutdown() {
    RoverIf::servos.set(RoverIf::ThrottleChannel, Servos::Mid);
    RoverIf::supervisor_changed();
}

void RoverIf::tick() {
    if (imu_timer.tick(50)) {
        defer(Pending::IMU);
    }

    pwmin_limiter.tick();
    RoverIf::supervisor.tick();

    if (blinker_timer.tick(1000/7)) {
        RoverIf::blinker.tick();
    }
    if (state_timer.tick(500)) {
        defer(Pending::State);
    }
    if (heartbeat_timer.tick(500)) {
        defer(Pending::Heartbeat);
        defer(Pending::Counters);
    }
    if (check_timer.tick(500)) {
        imu.check();
    }
}

#define DISPATCH_REQUEST(_name) \
    case Protocol::Code::_name: defer(Pending::_name)

void RoverIf::handle_request(const Protocol::Request& msg) {
    switch (msg.requested) {
        DISPATCH_REQUEST(Pong);
        DISPATCH_REQUEST(Version);
    default:
        break;
    }
}

void RoverIf::handle_demand(const Protocol::Demand& msg) {
    supervisor.update_pilot(msg.flags == Protocol::Demand::Flags::TakeControl);
    update_servos(msg.channels, true);
}

#define DISPATCH_MSG(_type, _handler) \
    case Protocol::Code::_type: \
    if (length == sizeof(Protocol::_type)) \
        _handler(*(const Protocol::_type*)p); \
    break

void RoverIf::poll_link() {
    uint8_t length;
    const void* p = link.peek(&length);

    if (p != nullptr) {
        switch (*(const Protocol::Code*)p) {
            DISPATCH_MSG(Request, handle_request);
            DISPATCH_MSG(Demand, handle_demand);
        default:
            break;
        }
        link.discard();
    }
}

void RoverIf::poll_ticks() {
    if (ticks_ != HAL::ticks) {
        ticks_++;

        static_assert((HAL::TickPrescaler & (HAL::TickPrescaler-1)) == 0,
                      "Tick prescaler must be a power of 2.");
        if ((ticks_ & (HAL::TickPrescaler-1)) == 0) {
            tick();
        }
    }
}

void RoverIf::poll_pwmin() {
    uint8_t cycles = pwmin.cycles;
    if (cycles != pwmin_cycles_) {
        pwmin_cycles_ = cycles;

        if (pwmin.get(ShutdownChannel) < -PWMIn::Full*2/3) {
            // Treat as missing.
        } else {
            supervisor.update_remote(
                abs(pwmin.get(ThrottleChannel)) > PWMIn::Full/5,
                pwmin.get(SwitchChannel) > -PWMIn::Full/2);

            int8_t channels[PWMIn::NumChannels];
            pwmin.get_all(channels);
            update_servos(channels, false);

            if (!pwmin_limiter.running()) {
                pwmin_limiter.start(50);
                defer(Pending::PWMIn);
            }
        }
    }
}

#define DISPATCH_PENDING(_code, _type, _handler) \
    case Pending::_code:                         \
    _handler((Protocol::_type*)pmsg);            \
    link.send(sizeof(Protocol::_type));          \
    break

#define DISPATCH_PENDINGIF(_code, _type, _handler) \
    case Pending::_code:                           \
    if (_handler((Protocol::_type*)pmsg)) {        \
        link.send(sizeof(Protocol::_type));        \
    }                                              \
    break

void RoverIf::poll_pending() {
    if (!pending_.is_empty()) {
        void* pmsg = link.start();

        if (pmsg != nullptr) {
            Pending next = (Pending)pending_.pop();
            switch (next) {
                DISPATCH_PENDING(PWMIn, Input, fill_pwmin);
                DISPATCH_PENDINGIF(IMU, IMU, fill_imu);
                DISPATCH_PENDING(State, State, fill_state);
                DISPATCH_PENDING(Heartbeat, Heartbeat, fill_heartbeat);
                DISPATCH_PENDING(Pong, Pong, fill_pong);
                DISPATCH_PENDING(Counters, Counters, fill_counters);
                DISPATCH_PENDING(Version, Version, fill_version);
            default:
                assert(false);
            }
        }
    }
}

void RoverIf::poll() {
    poll_link();
    poll_ticks();
    poll_pwmin();
    poll_pending();
}

void RoverIf::init() {
    // Need at least 10 ms resolution.
    static_assert(HAL::TicksPerSecond >= 100, "Tick rate is too low.");
    // Need to count to at least half a second.
    static_assert(HAL::TicksPerSecond <= 256*2, "Tick rate is too high.");
    static_assert(int(Pending::Max) <= 8, "Too many Pending entries.");

    HAL::init();
    servos.init();
    pwmin.init();
    imu.init();
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

int main() {
    RoverIf::init();
    RoverIf::run();
    return 0;
}
