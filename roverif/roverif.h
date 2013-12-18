#include <switcher.h>
#include <timer.h>
#include <usblink.h>
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

class RoverIf
{
public:
    static void init();
    static void run();

    static void poll();
    static void tick();

    static PWMIn pwmin;
    static Link link;
    static Switcher switcher;
    static Blinker blinker;
    static Supervisor supervisor;

private:
    enum class Pending : int8_t {
        PWMIn,
        Heartbeat,
        Pong,
    };

    static MinBitArray pending;

    static void defer(Pending event) { pending.set((int)event); }

    static void send_heartbeat(Protocol::Heartbeat& msg);
    static void send_pwmin(Protocol::Inputs& msg);
    static void send_pong(Protocol::Pong& msg);

    static void handle_ping(const Protocol::Ping& msg);

    static uint8_t ticks_;
};
