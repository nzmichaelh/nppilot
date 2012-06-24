#
# Makefile for some simple CopterControl programs.
#
# Michael Hope <michaelh@juju.net.nz>
#

CROSS_COMPILE = arm-none-eabi-

CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -mcpu=cortex-m3 -mthumb -O -nostdlib -std=gnu99 -g
CFLAGS += -Ilibmaple -DMCU_STM32F103CB -DSTM32_MEDIUM_DENSITY

all: blink.bin

blink.elf: blink.o
	$(CC) $(CFLAGS) -T flash.ld -o $@ $<

%.bin: %.elf
	$(OBJCOPY) -Obinary $< $@

clean:
	rm -f *.o *.elf *.bin
