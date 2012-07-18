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
#include "libmaple/usb_cdcacm.h"
#include "libmaple/usb/stm32f1/usb_reg_map.h"

#include <switcher.h>
#include <timer.h>
#include <platform/stm32/vectors.h>

extern uint8_t __bss_start;
extern uint8_t __bss_end;

extern const uint8_t __data_load;
extern uint8_t __data_start;
extern uint8_t __data_end;

void init();
int main();

static const int AHBClock = 72000000;
static const int APB1Clock = AHBClock / 2;

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

static void systick()
{
    switcher.trigger(1);
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

    timer_init(TIMER4);

    timer_gen_reg_map& regs = *TIMER4->regs.gen;

    /* For PWM mode we slave two capture units.  The first measures
     * the period and resets the counter and the second measures the
     * high time
     */
    
    /* TI1 is the input for both */
    regs.CCMR1 &= ~(TIMER_CCMR1_CC1S | TIMER_CCMR1_CC2S);
    regs.CCMR1 |= TIMER_CCMR1_CC1S_INPUT_TI1 | TIMER_CCMR1_CC2S_INPUT_TI1;

    /* Capture and reset on the rising edge */
    regs.CCER &= ~TIMER_CCER_CC1P;

    /* Capture on the falling edge */
    regs.CCER |= TIMER_CCER_CC2P;

    /* Set the trigger and reset methods */
    regs.SMCR &= ~(TIMER_SMCR_TS | TIMER_SMCR_SMS);
    regs.SMCR |= TIMER_SMCR_TS_TI1FP1 | TIMER_SMCR_SMS_RESET;
    
    /* Enable the capture units */
    regs.CCER |= TIMER_CCER_CC1E | TIMER_CCER_CC2E;

    /* Enable the main counter */
    regs.PSC = APB1Clock / 1000000;
    regs.CR1 |= TIMER_CR1_CEN;

    init_usb();
}

Switcher switcher;

static void heartbeat()
{
    usb_cdcacm_tx((const uint8_t*)" <3", 3);
}

static void blink()
{
    GPIOA_BASE->ODR ^= 1 << 6;
}

static void tick()
{
    Timer::tick_all();
}

MAKE_TIMER(expired, -1, Timer::Stopped);

static int16_t ch1;

static void poll()
{
    timer_gen_reg_map& regs = *TIMER4->regs.gen;

    uint32_t sr = regs.SR;

    if ((sr & TIMER_SR_CC2IF) != 0) {
        ch1 = regs.CCR2;
        timer_expired.start(1000);
    }

    if (!timer_expired.running()) {
        ch1 = -1;
    }

    usart_putudec(USART2, ch1);
    usart_putstr(USART2, "\r\n");
}

MAKE_TIMER(blink, 0, 200);
MAKE_TIMER(heartbeat, 2, 1000);
MAKE_TIMER(poll, 3, 100);

const Switcher::thread_entry Switcher::_dispatch[] =
{
    blink,
    tick,
    heartbeat,
    poll,
};

int main()
{
    for (;;) {
        switcher.next();
        asm ("wfi");
    }

    return 0;
}

/** Main function.  Called by the startup code. */
void _start()
{
  nvic_globalirq_disable();
  nvic_init((uint32_t)&vectors, 0);

  memcpy(&__data_start, &__data_load, &__data_end - &__data_start);
  memset(&__bss_start, 0, &__bss_end - &__bss_start);

  init();

  nvic_globalirq_enable();
  main();

  for (;;) {
  }
}

__attribute__((section(".vectors")))
const struct Vectors vectors =
{
  .stack_top = &__stack_top,
  .reset = _start,

  nullptr, nullptr, nullptr, nullptr, nullptr,
  {},
  .svc = nullptr,
  nullptr, 0, nullptr,

  .systick = __exc_systick,

  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,

  .usb_lp_can_rx0 = __irq_usb_lp_can_rx0,

  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,
  nullptr,

  .usart3 = __irq_usart3,
};
