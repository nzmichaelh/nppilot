#include <cstring>
#include <cstdint>

#include "roverif.h"

#include <vectors.h>

#include "libmaple/nvic.h"
#include "libmaple/usart.h"

extern uint8_t __bss_start;
extern uint8_t __bss_end;

extern const uint8_t __data_load;
extern uint8_t __data_start;
extern uint8_t __data_end;

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
