#include <cstring>
#include <cstdint>

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

extern "C" int main(void);

/** Main function.  Called by the startup code. */
void _start()
{
  nvic_globalirq_disable();
  nvic_init((uint32_t)&vectors, 0);

  memcpy(&__data_start, &__data_load, &__data_end - &__data_start);
  memset(&__bss_start, 0, &__bss_end - &__bss_start);

  for (init_function* i = &__init_array_start; i < &__init_array_end; i++) {
      (**i)();
  }

  nvic_globalirq_enable();
  main();

  for (;;) {
  }
}

__attribute__((naked))
static void irq_default()
{
  for (;;) {}
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
  irq_default,
};
