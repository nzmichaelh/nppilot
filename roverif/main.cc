#include <cstring>
#include <cstdint>


#include "libmaple/scb.h"
#include "libmaple/timer.h"
#include "libmaple/usart.h"
#include "libmaple/gpio.h"
#include "libmaple/iwdg.h"
#include "libmaple/usb_cdcacm.h"
#include "libmaple/usb/stm32f1/usb_reg_map.h"
#include "libmaple/systick.h"

#include <switcher.h>
#include <timer.h>
#include <cobs.h>
#include <usblink.h>
#include <platform/stm32/vectors.h>
#include <switch.h>
#include "supervisor.h"

#include "roverif.h"
#include "protocol.h"
#include "pwmin.h"
#include "blinker.h"

volatile int stuck;

void init();
int main();

static PWMIn pwmin;
static Switcher switcher;
static USBLink usblink;
static COBSLink link;
static Blinker blinker;
static Supervisor supervisor;

void irq_timer4ch1()
{
    timer_gen_reg_map& regs = *TIMER4->regs.gen;
    pwmin.irq(0, regs.CCR1, &regs.CCER, TIMER_CCER_CC1P);
}

void irq_timer3ch2()
{
    timer_gen_reg_map& regs = *TIMER3->regs.gen;
    pwmin.irq(1, regs.CCR2, &regs.CCER, TIMER_CCER_CC2P);
}

void irq_timer3ch3()
{
    timer_gen_reg_map& regs = *TIMER3->regs.gen;
    pwmin.irq(2, regs.CCR3, &regs.CCER, TIMER_CCER_CC3P);
}

void irq_timer3ch4()
{
    timer_gen_reg_map& regs = *TIMER3->regs.gen;
    pwmin.irq(3, regs.CCR4, &regs.CCER, TIMER_CCER_CC4P);
}

void irq_timer2ch1()
{
    timer_gen_reg_map& regs = *TIMER2->regs.gen;
    pwmin.irq(4, regs.CCR1, &regs.CCER, TIMER_CCER_CC1P);
}

void irq_timer2ch2()
{
    timer_gen_reg_map& regs = *TIMER2->regs.gen;
    pwmin.irq(5, regs.CCR2, &regs.CCER, TIMER_CCER_CC2P);
}

static void put_hex(uint32_t v)
{
    static const char* chars = "0123456789abcedf";

    for (int i = 32-4; i >= 0; i -= 4) {
        usart_putc(USART2, chars[(v >> i) & 0x0f]);
    }
}

__attribute__((noinline))
static void _systick(uint32_t* sp)
{
    if (++stuck == 1000) {
        usart_putstr(USART2, "\r\nstuck at ");
        put_hex(sp[6]);
        usart_putstr(USART2, "\r\n");
    }

    switcher.trigger(SysTickID);
}

void systick()
{
    uint32_t* sp;
    asm volatile ("mov %0, sp" : "=r" (sp));
    _systick(sp);
}


void COBSLink::write(const uint8_t* p, int length)
{
    usblink.write(p, length);
}

void Timer::dispatch(int id)
{
    switcher.trigger(id);
}

void COBSLink::dispatch(int id, const void* msg, int length)
{
    usart_putstr(USART2, "COBSLink::dispatch\r\n");
}

static void heartbeat()
{
    Protocol::Heartbeat msg = { .version = 1, .device_id = 2 };
    link.send('h', &msg, sizeof(msg));
}

void Blinker::update(bool level)
{
    if (level) {
        GPIOA_BASE->ODR &= ~(1 << 6);
    } else {
        GPIOA_BASE->ODR |= 1 << 6;
    }
}

void Supervisor::changed()
{
    switch (supervisor.state()) {
    case Supervisor::State::Remote:
        blinker.set(0b100000101);
        break;
    case Supervisor::State::RemoteArmed:
        blinker.set(0b100001101);
        break;
    case Supervisor::State::Pilot:
        blinker.set(0b111111110);
        break;
    case Supervisor::State::Shutdown:
        blinker.set(0b100000001);
        break;
    default:
        blinker.set(0b101);
        break;
    }
}

static void tick()
{
    Timer::tick_all();
}

MAKE_TIMER(timer_expired, -1, Timer::Stopped);
MAKE_TIMER(blinker_tick, BlinkerID, 100);
 
static void poll()
{
    static int at;
    static int direction = 1;

    iwdg_feed();
    stuck = 0;

    Protocol::Inputs msg = { };

    for (int i = 0; i < pwmin.count(); i++) {
        msg.channel[i] = pwmin.value(i);
    }

    supervisor.set_remote(msg.channel, 6);

    int ch1 = pwmin.value(0) - 8450;
    int out = 0;
    // 10550 8450 6300

    timer_set_compare(TIMER4, 4, 8450 + ch1);

    link.send('i', &msg, sizeof(msg));
    
    if (usb_cdcacm_data_available()) {
        uint8_t rx[32];
        int got = usb_cdcacm_rx(rx, sizeof(rx));

        link.feed(rx, got);
    }
}

MAKE_TIMER(timer_heartbeat, HeartbeatID, 1000);
MAKE_TIMER(timer_poll, PollID, 20);

// 8296 counts as throttle cut

void Switcher::dispatch(int id)
{
    switch (id) {
    case BlinkerID:
        blinker.tick();
        break;
    case SysTickID:
        tick();
        break;
    case HeartbeatID:
        heartbeat();
        break;
    case PollID:
        poll();
        break;
    case SupervisorID:
        supervisor.expired();
        break;
    default:
        assert(false);
        break;
    }
}

extern "C" usart_dev* __lm_enable_error_usart(void)
{
    return USART2;
}

__attribute__((noinline))
void stick()
{
    for (;;) {}
}

int main()
{
    blinker.set(0b101);
//    stick();

    usart_putstr(USART2, "\r\n\r\ngo\r\n");
    usart_putudec(USART2, *(uint32_t*)0xE000ED00);

    for (;;) {
        switcher.next();
        usblink.flush();
        asm ("wfi");
    }

    return 0;
}

