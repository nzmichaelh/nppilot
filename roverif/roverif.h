#include <switcher.h>
#include <timer.h>
#include <cobs.h>
#include <usblink.h>
#include <switch.h>
#include "supervisor.h"
#include <debug.h>

#include "protocol.h"
#include "pwmin.h"
#include "blinker.h"

extern void systick();
void irq_timer4ch1();
void irq_timer3ch2();
void irq_timer3ch3();
void irq_timer3ch4();
void irq_timer2ch1();
void irq_timer2ch2();

void init();
int main();

enum ThreadID {
    BlinkerID,
    SysTickID,
    HeartbeatID,
    PollID,
    SupervisorID,
};

class RoverIf
{
public:
    static void init();
    static void run();

    static void poll();
    static void tick();

    static PWMIn pwmin;
    static Switcher switcher;
    static USBLink usblink;
    static COBSLink link;
    static Blinker blinker;
    static Supervisor supervisor;
};
