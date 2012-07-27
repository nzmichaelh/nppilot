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

#include "protocol.h"
#include "pwmin.h"
#include "blinker.h"

volatile int stuck;

void init();
int main();

enum ThreadID {
    BlinkerID,
    SysTickID,
    HeartbeatID,
    PollID,
};

static PWMIn pwmin;
static Switcher switcher;
static USBLink usblink;
static COBSLink link;
static Blinker blinker;

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

static void _systick(uint32_t* sp)
{
    if (++stuck == 1000) {
        usart_putstr(USART2, "\r\nstuck at ");
        put_hex(sp[8]);
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

static void tick()
{
    Timer::tick_all();
}

MAKE_TIMER(timer_expired, -1, Timer::Stopped);
MAKE_TIMER(blinker_tick, BlinkerID, 100);
 
static int get_switch(int level) {
    if (level <= (9160 + 6310)/2) {
        return -1;
    } else if (level <= (12000 + 9160)/2) {
        return 0;
    } else {
        return 1;
    }
}

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

    int ch1 = pwmin.value(0) - 8450;
    int out = 0;
    // 10550 8450 6300

    switch (get_switch(pwmin.value(4))) {
    case -1:
        ch1 = -ch1;
        break;
    case 0: {
        int step = pwmin.value(5) - 6500;
        if (step < 0) step = 0;
        step /= 16;
        step *= direction;

        at += step;
        if (at > 2000) {
            direction = -1;
        } else if (at < -2000) {
            direction = 1;
        }

        ch1 = at;
        break;
    }
    case 1:
        ch1 = +ch1;
        break;
    }

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
    default:
        assert(false);
        break;
    }
}

int main()
{
    blinker.set(~0 ^ 1 ^ 4 ^ 32);
    usart_putstr(USART2, "\r\n\r\ngo\r\n");
    usart_putudec(USART2, *(uint32_t*)0xE000ED00);

    for (;;) {
        switcher.next();
        usblink.flush();
        asm ("wfi");
    }

    return 0;
}

