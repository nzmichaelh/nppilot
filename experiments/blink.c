/*
 * Blink the status LED on the CopterControl.
 *
 * Michael Hope <michaelh@juju.net.nz>
 *
 */

#include <stdint.h>

#include "libmaple/rcc.h"
#include "libmaple/gpio.h"

/* GPIOC12 for the H103, GPIOA6 for the CopterControl. */
#define PORT GPIOA_BASE

/* Optimization barrier */
#define barrier() __asm__ __volatile__("": : :"memory")

void _start();
extern uint32_t __stack_top;

/* Main vector table.  Pulled in by the linker script. */
__attribute__((section(".vectors")))
const void *vectors[] =
{
  &__stack_top,
  _start,
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
void _start()
{
    // Most of the peripherals are connected to APB2.  Turn on the
    // clocks for the interesting peripherals
    RCC_BASE->APB2ENR = 0
        // Turn on USART1
        | RCC_APB2ENR_USART1EN
        // Turn on IO Ports A, B, and C
        | RCC_APB2ENR_IOPAEN
        | RCC_APB2ENR_IOPBEN
        | RCC_APB2ENR_IOPCEN
        // Turn on the alternate function block
        | RCC_APB2ENR_AFIOEN;

    // Put all pins 15 into open drain output function/50 MHz
    // mode.
    PORT->CRL = 0x33333333;
    PORT->CRH = 0x33333333;

    for (;;)
    {
        delay(1000);
        PORT->ODR = ~0;

        delay(500);
        PORT->ODR = 0;
    }
}
