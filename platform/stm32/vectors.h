#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef void (*interrupt_handler)();

extern uint32_t __stack_top;

struct Vectors
{
    uint32_t *stack_top;
    interrupt_handler reset;
    interrupt_handler nmi;
    interrupt_handler hard_fault;
    interrupt_handler mem_manage;
    interrupt_handler bus_fault;
    interrupt_handler usage_fault;

    uint32_t reserved[4];

    interrupt_handler svc;
    interrupt_handler debug_monitor;

    uint32_t reserved2;

    interrupt_handler pendsv;
    interrupt_handler systick;

    interrupt_handler wwdg;
    interrupt_handler pvd;
    interrupt_handler tamper;
    interrupt_handler rtc;
    interrupt_handler flash;
    interrupt_handler rcc;
    interrupt_handler exti0;
    interrupt_handler exti1;
    interrupt_handler exti2;
    interrupt_handler exti3;
    interrupt_handler exti4;
    interrupt_handler dma1_channel1;
    interrupt_handler dma1_channel2;
    interrupt_handler dma1_channel3;
    interrupt_handler dma1_channel4;
    interrupt_handler dma1_channel5;
    interrupt_handler dma1_channel6;
    interrupt_handler dma1_channel7;
    interrupt_handler adc;
    interrupt_handler usb_hp_can_tx;
    interrupt_handler usb_lp_can_rx0;
    interrupt_handler can_rx1;
    interrupt_handler can_sce;
    interrupt_handler exti9_5;
    interrupt_handler tim1_brk;
    interrupt_handler tim1_up;
    interrupt_handler tim1_trg_com;
    interrupt_handler tim1_cc;
    interrupt_handler tim2;
    interrupt_handler tim3;
    interrupt_handler tim4;
    interrupt_handler i2c1_ev;
    interrupt_handler i2c1_er;
    interrupt_handler i2c2_ev;
    interrupt_handler i2c2_er;
    interrupt_handler spi1;
    interrupt_handler spi2;
    interrupt_handler usart1;
    interrupt_handler usart2;
    interrupt_handler usart3;
    interrupt_handler exti15_10;
    interrupt_handler rtcalarm;
    interrupt_handler usbwakeup;
    interrupt_handler tim8_brk;
    interrupt_handler tim8_up;
    interrupt_handler tim8_trg_com;
    interrupt_handler tim8_cc;
    interrupt_handler adc3;
    interrupt_handler fsmc;
    interrupt_handler sdio;
    interrupt_handler tim5;
    interrupt_handler spi3;
    interrupt_handler uart4;
    interrupt_handler uart5;
    interrupt_handler tim6;
    interrupt_handler tim7;
    interrupt_handler dma2_channel1;
    interrupt_handler dma2_channel2;
    interrupt_handler dma2_channel3;
    interrupt_handler dma2_channel4_5;
};

extern const struct Vectors vectors;

void _start();
void __irq_usb_lp_can_rx0(void);
void __irq_usart3(void);
void __exc_systick(void);
void __exc_svc(void);

#ifdef __cplusplus
}
#endif
