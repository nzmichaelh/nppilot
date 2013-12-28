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

void systick();
void irq_timer4ch1();
void irq_timer3ch2();
void irq_timer3ch3();
void irq_timer3ch4();
void irq_timer2ch1();
void irq_timer2ch2();

void init();
int main();

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
        Heartbeat,
        Pong,
        Version,
    };

    static MinBitArray pending;

    static void defer(Pending event) { pending.set(static_cast<int>(event)); }

    static void fill_heartbeat(Protocol::Heartbeat* pmsg);
    static void fill_pwmin(Protocol::Inputs* pmsg);
    static void fill_pong(Protocol::Pong* pmsg);
    static void fill_version(Protocol::Version* pmsg);

    static void handle_request(const Protocol::Request& msg);

    static uint8_t ticks_;
};
