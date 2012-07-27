#include <cstring>
#include <cstdint>

#include "libmaple/rcc.h"
#include "libmaple/gpio.h"
#include "libmaple/systick.h"
#include "libmaple/nvic.h"
#include "libmaple/scb.h"
#include "libmaple/flash.h"
#include "libmaple/usart.h"
#include "libmaple/delay.h"
#include "libmaple/timer.h"
#include "libmaple/iwdg.h"
#include "libmaple/usb_cdcacm.h"
#include "libmaple/usb/stm32f1/usb_reg_map.h"

#include <switcher.h>
#include <timer.h>
#include <cobs.h>
#include <usblink.h>
#include <platform/stm32/vectors.h>

#include "protocol.h"

extern uint8_t __bss_start;
extern uint8_t __bss_end;

extern const uint8_t __data_load;
extern uint8_t __data_start;
extern uint8_t __data_end;

volatile int stuck;

void init();
int main();

static const int AHBClock = 72000000;
static const int APB1Clock = AHBClock / 2;

class Inputs
{
public:
    Inputs();

    int capacity() const { return Capacity; }
    uint16_t value(int channel) const { return value_[channel]; }

    void irq(int channel, uint32_t ccr, __io uint32_t* ccer, int mask);

private:
    static const int Capacity = 6;

    uint16_t value_[Capacity];
    uint16_t last_[Capacity];
};

Inputs::Inputs()
{
}

void Inputs::irq(int channel, uint32_t ccr, __io uint32_t* pccer, int mask)
{
    uint32_t ccer = *pccer;
    *pccer = ccer ^ mask;

    if ((ccer & mask) == 0) {
        value_[channel] = ccr - last_[channel];
    }

    last_[channel] = ccr;
}

static Inputs inputs;
static Switcher switcher;
static USBLink usblink;
static COBSLink link;

static void irq_timer4ch1()
{
    timer_gen_reg_map& regs = *TIMER4->regs.gen;
    inputs.irq(0, regs.CCR1, &regs.CCER, TIMER_CCER_CC1P);
}

static void irq_timer3ch2()
{
    timer_gen_reg_map& regs = *TIMER3->regs.gen;
    inputs.irq(1, regs.CCR2, &regs.CCER, TIMER_CCER_CC2P);
}

static void irq_timer3ch3()
{
    timer_gen_reg_map& regs = *TIMER3->regs.gen;
    inputs.irq(2, regs.CCR3, &regs.CCER, TIMER_CCER_CC3P);
}

static void irq_timer3ch4()
{
    timer_gen_reg_map& regs = *TIMER3->regs.gen;
    inputs.irq(3, regs.CCR4, &regs.CCER, TIMER_CCER_CC4P);
}

static void irq_timer2ch1()
{
    timer_gen_reg_map& regs = *TIMER2->regs.gen;
    inputs.irq(4, regs.CCR1, &regs.CCER, TIMER_CCER_CC1P);
}

static void irq_timer2ch2()
{
    timer_gen_reg_map& regs = *TIMER2->regs.gen;
    inputs.irq(5, regs.CCR2, &regs.CCER, TIMER_CCER_CC2P);
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

    switcher.trigger(1);
}

static void systick()
{
    uint32_t* sp;

    asm volatile ("mov %0, sp" : "=r" (sp));
    _systick(sp);
}

void init_usb()
{
  rcc_set_prescaler(RCC_PRESCALER_USB, 0);
  rcc_clk_enable(RCC_USB);
  rcc_reset_dev(RCC_USB);

  /* Force USB reset and power-down (this will also release the USB pins for direct GPIO control) */
  USB_BASE->CNTR = USB_CNTR_FRES | USB_CNTR_PDWN;

  /* Using a "dirty" method to force a re-enumeration: */
  /* Force DPM (Pin PA12) low for ca. 10 mS before USB Tranceiver will be enabled */
  /* This overrules the external Pull-Up at PA12, and at least Windows & MacOS will enumerate again */
  gpio_set_mode(GPIOA, 12, GPIO_OUTPUT_PP);
  gpio_write_bit(GPIOA, 12, 0);

  delay_us(10000);

  /* Release power-down, still hold reset */
  USB_BASE->CNTR = USB_CNTR_PDWN;
  delay_us(1000);

  /* CNTR_FRES = 0 */
  USB_BASE->CNTR = 0;

  /* Clear pending interrupts */
  USB_BASE->ISTR = 0;

  gpio_set_mode(GPIOA, 12, GPIO_AF_OUTPUT_OD);

  usb_cdcacm_enable(GPIOB, 10);
}

void init_timers()
{
    timer_init(TIMER4);
    timer_init(TIMER3);
    timer_init(TIMER2);

    timer_gen_reg_map& regs4 = *TIMER4->regs.gen;

    /* For PWM mode we slave two capture units.  The first measures
     * the period and resets the counter and the second measures the
     * high time
     */
    
    /* TI1 is the input for both */
    regs4.CCMR1 &= ~TIMER_CCMR1_CC1S;
    regs4.CCMR1 |= TIMER_CCMR1_CC1S_INPUT_TI1;

    /* Capture on the falling edge */
    regs4.CCER |= TIMER_CCER_CC1P;
    
    /* Enable the capture unit */
    regs4.CCER |= TIMER_CCER_CC1E;

    /* The Futaba T7C has a period of 16 ms.  The internet says that
     * some have a period of 20 ms.  Do a reload of 40 ms.
     */
    regs4.PSC = APB1Clock / 65536 / 40;
    /* Enable the main counter */
    regs4.CR1 |= TIMER_CR1_CEN;

    timer_attach_interrupt(TIMER4, TIMER_CC1_INTERRUPT, irq_timer4ch1);

    timer_set_mode(TIMER4, TIMER_CH4, TIMER_PWM);
    timer_set_compare(TIMER4, 4, 8000);
    gpio_set_mode(GPIOB, 9, GPIO_AF_OUTPUT_PP);

    AFIO_BASE->MAPR |= AFIO_MAPR_TIM3_REMAP_PARTIAL;

    timer_gen_reg_map& regs3 = *TIMER3->regs.gen;

    /* For PWM mode we slave two capture units.  The first measures
     * the period and resets the counter and the second measures the
     * high time
     */
    
    /* TI1 is the input for both */
    regs3.CCMR1 &= ~TIMER_CCMR1_CC2S;
    regs3.CCMR1 |= TIMER_CCMR1_CC2S_INPUT_TI2;

    regs3.CCMR2 &= ~TIMER_CCMR2_CC3S;
    regs3.CCMR2 |= TIMER_CCMR2_CC3S_INPUT_TI3;
    regs3.CCMR2 &= ~TIMER_CCMR2_CC4S;
    regs3.CCMR2 |= TIMER_CCMR2_CC4S_INPUT_TI4;

    /* Capture on the falling edge */
    regs3.CCER |= TIMER_CCER_CC2P | TIMER_CCER_CC3P | TIMER_CCER_CC4P;
    
    /* Enable the capture unit */
    regs3.CCER |= TIMER_CCER_CC2E | TIMER_CCER_CC3E | TIMER_CCER_CC4E;

    /* The Futaba T7C has a period of 16 ms.  The internet says that
     * some have a period of 20 ms.  Do a reload of 40 ms.
     */
    regs3.PSC = APB1Clock / 65536 / 40;
    /* Enable the main counter */
    regs3.CR1 |= TIMER_CR1_CEN;

    timer_attach_interrupt(TIMER3, TIMER_CC2_INTERRUPT, irq_timer3ch2);
    timer_attach_interrupt(TIMER3, TIMER_CC3_INTERRUPT, irq_timer3ch3);
    timer_attach_interrupt(TIMER3, TIMER_CC4_INTERRUPT, irq_timer3ch4);

    timer_gen_reg_map& regs2 = *TIMER2->regs.gen;

    /* For PWM mode we slave two capture units.  The first measures
     * the period and resets the counter and the second measures the
     * high time
     */
    
    regs2.CCMR1 &= ~TIMER_CCMR1_CC1S;
    regs2.CCMR1 |= TIMER_CCMR1_CC1S_INPUT_TI1;
    regs2.CCMR1 &= ~TIMER_CCMR1_CC2S;
    regs2.CCMR1 |= TIMER_CCMR1_CC2S_INPUT_TI2;

    /* Capture on the falling edge */
    regs2.CCER |= TIMER_CCER_CC1P | TIMER_CCER_CC2P;
    
    /* Enable the capture unit */
    regs2.CCER |= TIMER_CCER_CC1E | TIMER_CCER_CC2E;

    /* The Futaba T7C has a period of 16 ms.  The internet says that
     * some have a period of 20 ms.  Do a reload of 40 ms.
     */
    timer_set_prescaler(TIMER2, APB1Clock / 65536 / 40);

    /* Enable the main counter */
    regs2.CR1 |= TIMER_CR1_CEN;

    timer_attach_interrupt(TIMER2, TIMER_CC1_INTERRUPT, irq_timer2ch1);
    timer_attach_interrupt(TIMER2, TIMER_CC2_INTERRUPT, irq_timer2ch2);
}

void init()
{
    flash_enable_prefetch();
    flash_set_latency(FLASH_WAIT_STATE_2);

    rcc_clk_init(RCC_CLKSRC_PLL, RCC_PLLSRC_HSE, RCC_PLLMUL_9);
    rcc_set_prescaler(RCC_PRESCALER_AHB, RCC_AHB_SYSCLK_DIV_1);
    rcc_set_prescaler(RCC_PRESCALER_APB1, RCC_APB1_HCLK_DIV_2);
    rcc_set_prescaler(RCC_PRESCALER_APB2, RCC_APB2_HCLK_DIV_1);

    systick_init(72000 - 1);
    systick_attach_callback(systick);

    gpio_init_all();

    usart_init(USART2);
    usart_set_baud_rate(USART2, 72000000/2, 115200);
    usart_enable(USART2);

    gpio_set_mode(GPIOA, 2, GPIO_AF_OUTPUT_PP);
    gpio_set_mode(GPIOA, 6, GPIO_OUTPUT_PP);

    init_timers();
    init_usb();

    /* Reset after 500 ms */
    iwdg_init(IWDG_PRE_32, 40000 / 32 * 2000/1000);
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

static void blink()
{
    GPIOA_BASE->ODR ^= 1 << 6;
}

static void tick()
{
    Timer::tick_all();
}

MAKE_TIMER(timer_expired, -1, Timer::Stopped);

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

    for (int i = 0; i < inputs.capacity(); i++) {
        msg.channel[i] = inputs.value(i);
    }

    int ch1 = inputs.value(0) - 8450;
    int out = 0;
    // 10550 8450 6300

    switch (get_switch(inputs.value(4))) {
    case -1:
        ch1 = -ch1;
        break;
    case 0: {
        int step = inputs.value(5) - 6500;
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

MAKE_TIMER(timer_blink, 0, 200);
MAKE_TIMER(timer_heartbeat, 2, 1000);
MAKE_TIMER(timer_poll, 3, 20);

void Switcher::dispatch(int id)
{
    switch (id) {
    case 0:
        blink();
        break;
    case 1:
        tick();
        break;
    case 2:
        heartbeat();
        break;
    case 3:
        poll();
        break;
    default:
        assert(false);
        break;
    }
}

int main()
{
    systick_uptime_millis = 0;

    usart_putstr(USART2, "\r\n\r\ngo\r\n");
    usart_putudec(USART2, *(uint32_t*)0xE000ED00);

    for (;;) {
        switcher.next();
        usblink.flush();
        asm ("wfi");
    }

    return 0;
}

typedef void (*init_function)(void);

extern init_function __init_array_start;
extern init_function __init_array_end;

/** Main function.  Called by the startup code. */
void _start()
{
  nvic_globalirq_disable();
  nvic_init((uint32_t)&vectors, 0);

  memcpy(&__data_start, &__data_load, &__data_end - &__data_start);
  memset(&__bss_start, 0, &__bss_end - &__bss_start);

  init();

  for (init_function* i = &__init_array_start; i < &__init_array_end; i++) {
      (**i)();
  }

  nvic_globalirq_enable();
  main();

  for (;;) {
  }
}

static void _irq_default(uint32_t* sp)
{
    uint32_t icsr = *(uint32_t*)0xE000ED04;
    usart_putstr(USART2, "irq_default\r\n");
    usart_putudec(USART2, icsr & 0x1FF);
    usart_putstr(USART2, "\r\n\r\n");

    for (int i = 0; i < 10; i++) {
        usart_putudec(USART2, *sp--);
        usart_putstr(USART2, "\r\n");
    }

    for (;;) {}
}

__attribute__((naked))
static void irq_default()
{
    uint32_t* sp;

    asm volatile ("mov %0, sp" : "=r" (sp));
    _irq_default(sp);
}

__attribute__((section(".vectors")))
const struct Vectors vectors =
{
  .stack_top = &__stack_top,
  .reset = _start,

  irq_default, irq_default, irq_default, irq_default, irq_default,
  {},
  .svc = irq_default,
  irq_default, 0, irq_default,

  .systick = __exc_systick,

  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,

  .usb_lp_can_rx0 = __irq_usb_lp_can_rx0,

  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,
  irq_default,

  .tim2 = __irq_tim2,
  .tim3 = __irq_tim3,
  .tim4 = __irq_tim4,

  .i2c1_ev = irq_default,
  .i2c1_er = irq_default,
  .i2c2_ev = irq_default,
  .i2c2_er = irq_default,
  .spi1 = irq_default,
  .spi2 = irq_default,
  .usart1 = irq_default,
  .usart2 = __irq_usart2,
  .usart3 = __irq_usart3,
};
