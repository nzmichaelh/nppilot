/*
 * CopterControl specific implementation.
 */

#include "roverif.h"

#include "libmaple/scb.h"
#include "libmaple/timer.h"
#include "libmaple/usart.h"
#include "libmaple/gpio.h"
#include "libmaple/iwdg.h"
#include "libmaple/usb_cdcacm.h"
#include "libmaple/usb/stm32f1/usb_reg_map.h"
#include "libmaple/systick.h"

#include <platform/stm32/vectors.h>

volatile int stuck;


void irq_timer4ch1()
{
    timer_gen_reg_map& regs = *TIMER4->regs.gen;
    RoverIf::pwmin.irq(0, regs.CCR1, &regs.CCER, TIMER_CCER_CC1P);
}

void irq_timer3ch2()
{
    timer_gen_reg_map& regs = *TIMER3->regs.gen;
    RoverIf::pwmin.irq(1, regs.CCR2, &regs.CCER, TIMER_CCER_CC2P);
}

void irq_timer3ch3()
{
    timer_gen_reg_map& regs = *TIMER3->regs.gen;
    RoverIf::pwmin.irq(2, regs.CCR3, &regs.CCER, TIMER_CCER_CC3P);
}

void irq_timer3ch4()
{
    timer_gen_reg_map& regs = *TIMER3->regs.gen;
    RoverIf::pwmin.irq(3, regs.CCR4, &regs.CCER, TIMER_CCER_CC4P);
}

void irq_timer2ch1()
{
    timer_gen_reg_map& regs = *TIMER2->regs.gen;
    RoverIf::pwmin.irq(4, regs.CCR1, &regs.CCER, TIMER_CCER_CC1P);
}

void irq_timer2ch2()
{
    timer_gen_reg_map& regs = *TIMER2->regs.gen;
    RoverIf::pwmin.irq(5, regs.CCR2, &regs.CCER, TIMER_CCER_CC2P);
}

__attribute__((noinline))
static void _systick(uint32_t* sp)
{
    RoverIf::switcher.trigger(SysTickID);

    if (++stuck == 1000) {
        Debug::error("Stuck at %x", sp[6]);
    }
}

void systick()
{
    uint32_t* sp;
    asm volatile ("mov %0, sp" : "=r" (sp));
    _systick(sp);
}

void RoverIf::set_status_led(bool level)
{
    if (level) {
        GPIOA_BASE->ODR &= ~(1 << 6);
    } else {
        GPIOA_BASE->ODR |= 1 << 6;
    }
}

void RoverIf::poll_hal()
{
    if (usb_cdcacm_data_available()) {
        uint8_t rx[32];
        int got = usb_cdcacm_rx(rx, sizeof(rx));
        link.feed(rx, got);
    }

    iwdg_feed();
}    

void RoverIf::wait()
{
    asm ("wfi");
}

void Debug::putch(char ch)
{
}

static void init_usb()
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

static void init_timers()
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
    timer_set_prescaler(TIMER4, Board::APB1Clock / 65536 / 40);
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
    timer_set_prescaler(TIMER3, Board::APB1Clock / 65536 / 40);

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
    timer_set_prescaler(TIMER2, Board::APB1Clock / 65536 / 40);

    /* Enable the main counter */
    regs2.CR1 |= TIMER_CR1_CEN;

    timer_attach_interrupt(TIMER2, TIMER_CC1_INTERRUPT, irq_timer2ch1);
    timer_attach_interrupt(TIMER2, TIMER_CC2_INTERRUPT, irq_timer2ch2);
}

static void init_debug_usart()
{
    usart_init(USART3);
    usart_set_baud_rate(USART3, Board::APB1Clock, Board::DebugBaudRate);
    usart_enable(USART3);

    gpio_set_mode(GPIOA, 2, GPIO_AF_OUTPUT_PP);
    gpio_set_mode(GPIOA, 6, GPIO_OUTPUT_PP);
}

static void init_clocks()
{
    rcc_clk_init(RCC_CLKSRC_PLL, RCC_PLLSRC_HSE, RCC_PLLMUL_9);
    rcc_set_prescaler(RCC_PRESCALER_AHB, RCC_AHB_SYSCLK_DIV_1);
    rcc_set_prescaler(RCC_PRESCALER_APB1, RCC_APB1_HCLK_DIV_2);
    rcc_set_prescaler(RCC_PRESCALER_APB2, RCC_APB2_HCLK_DIV_1);
}

void RoverIf::init_hal()
{
    /* Reset after 500 ms */
    iwdg_init(IWDG_PRE_32, Board::LSIClock / 32 * 2000/1000);

    flash_enable_prefetch();
    flash_set_latency(FLASH_WAIT_STATE_2);
    init_clocks();
    init_debug_usart();

    systick_init(Board::AHBClock / Board::Ticks - 1);

    gpio_init_all();
    init_timers();
    init_usb();
}

void RoverIf::start_hal()
{
}
