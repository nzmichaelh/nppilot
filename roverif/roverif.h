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

    static Servos servos;
    static PWMIn pwmin;
    static Link link;
    static Blinker blinker;
    static Supervisor supervisor;

 private:
    enum class Pending : int8_t {
        PWMIn,
        State,
        Heartbeat,
        Pong,
        Version,
    };

    static MinBitArray pending;

    static void defer(Pending event) { pending.set(static_cast<int>(event)); }
    static bool tick_one(Timer& timer, int divisor);

    static void fill_heartbeat(Protocol::Heartbeat* pmsg);
    static void fill_pwmin(Protocol::Input* pmsg);
    static void fill_state(Protocol::State* pmsg);
    static void fill_pong(Protocol::Pong* pmsg);
    static void fill_version(Protocol::Version* pmsg);

    static void handle_request(const Protocol::Request& msg);
    static void handle_demand(const Protocol::Demand& msg);

    static uint8_t ticks_;
};
