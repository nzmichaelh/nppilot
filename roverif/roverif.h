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
    static void heartbeat();

    static PWMIn pwmin;
    static Link link;
    static Switcher switcher;
    static Blinker blinker;
    static Supervisor supervisor;

private:
    static uint8_t ticks_;
};
