#include <timer.h>
#include <switch.h>
#include "supervisor.h"
#include "link.h"
#include <debug.h>

#include "protocol.h"
#include "pwmin.h"
#include "blinker.h"
#include "servos.h"
#include "minbitarray.h"

class RoverIf {
 public:
    static void init();
    static void run();

    static void poll();
    static void tick();

    static void supervisor_changed();

    static Servos servos;
    static PWMIn pwmin;
    static Link link;
    static Blinker blinker;
    static Supervisor supervisor;

    static const int ThrottleChannel = 2;
    static const int ShutdownChannel = 3;
    static const int SwitchChannel = 4;

 private:
    enum class Pending {
        PWMIn,
        State,
        Heartbeat,
        Pong,
        Version,
    };

    static void defer(Pending event) { pending_.set(static_cast<int>(event)); }
    static bool tick_one(Timer* ptimer, int divisor);

    static void poll_pwmin();
    static void poll_link();
    static void poll_ticks();
    static void poll_pending();

    static void fill_heartbeat(Protocol::Heartbeat* pmsg);
    static void fill_pwmin(Protocol::Input* pmsg);
    static void fill_state(Protocol::State* pmsg);
    static void fill_pong(Protocol::Pong* pmsg);
    static void fill_version(Protocol::Version* pmsg);

    static void handle_request(const Protocol::Request& msg);
    static void handle_demand(const Protocol::Demand& msg);

    static void update_servos(const int8_t* pdemands, bool from_pilot);

    static uint8_t ticks_;
    static uint8_t pwmin_cycles_;
    static MinBitArray pending_;
    static uint8_t pilot_supplied_;
};
