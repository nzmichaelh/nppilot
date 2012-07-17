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

MAKE_TIMER(blink, 0, 200);
MAKE_TIMER(heartbeat, 2, 1000);

const Switcher::thread_entry Switcher::_dispatch[] =
{
    blink,
    tick,
    heartbeat,
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
