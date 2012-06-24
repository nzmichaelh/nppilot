/*
 * Blink the status LED on the CopterControl.
 *
 * Michael Hope <michaelh@juju.net.nz>
 *
 */

#include <stdint.h>

#include "libmaple/rcc.h"
#include "libmaple/gpio.h"

/* Optimization barrier */
#define barrier() __asm__ __volatile__("": : :"memory")

static void start();
extern uint32_t _stack_top;

__attribute__((section(".vectors")))
const void *vectors[] =
{
  &_stack_top,
  start,
};

/** Spin delay */
void delay(int count)
{
    for (int i = 0; i < 10000*count; i++)
    {
      barrier();
    }
}

/** Main function.  Called by the startup code. */
static void start()
{
    // Most of the peripherals are connected to APB2.  Turn on the
    // clocks for the interesting peripherals
    RCC_BASE->APB2ENR = 0
        // Turn on USART1
        | RCC_APB2ENR_USART1EN
        // Turn on IO Port A
        | RCC_APB2ENR_IOPAEN
        // Turn on IO Port B
        | RCC_APB2ENR_IOPBEN
        // Turn on the alternate function block
        | RCC_APB2ENR_AFIOEN;

    // Put Port A pins 8 through 15 into alternate function/50 MHz
    // mode.
    GPIOA_BASE->CRL = 0x33333333;
    GPIOA_BASE->CRH = 0x33333333;

    for (;;)
    {
        delay(1000);
        GPIOA_BASE->ODR = ~0;

        delay(0500);
        GPIOA_BASE->ODR = 0;
    }
}
