bin_PROGRAMS = blink.elf

blink_elf_SOURCES = \
	experiments/blink.c

roverif_elf_SOURCES = \
	roverif/main.cc \
	roverif/init.cc \
	roverif/start.cc \
	roverif/pwmin.cc \
	roverif/supervisor.cc

roverif_elf_LDADD = libnppilot.a libmaple.a libmaple-usb.a

check_PROGRAMS = \
	tests/runner

tests_runner_LDADD = libnppilot.a

lib_LIBRARIES = \
	libmaple.a \
	libmaple-usb.a \
	libnppilot.a

libnppilot_a_SOURCES = \
	lib/cobs.cc \
	lib/switcher.cc \
	lib/timer.cc \
	lib/blinker.cc \
	lib/switch.cc \
	platform/stm32/bitarray.cc \
	platform/stm32/timer.cc \
	platform/stm32/usblink.cc

#	platform/posix/bitarray.cc


libmaple_a_SOURCES = \
	external/libmaple/libmaple/adc.c \
	external/libmaple/libmaple/dac.c \
	external/libmaple/libmaple/dma.c \
	external/libmaple/libmaple/exti.c \
	external/libmaple/libmaple/flash.c \
	external/libmaple/libmaple/gpio.c \
	external/libmaple/libmaple/i2c.c \
	external/libmaple/libmaple/iwdg.c \
	external/libmaple/libmaple/nvic.c \
	external/libmaple/libmaple/pwr.c \
	external/libmaple/libmaple/rcc.c \
	external/libmaple/libmaple/spi.c \
	external/libmaple/libmaple/systick.c \
	external/libmaple/libmaple/timer.c \
	external/libmaple/libmaple/usart.c \
	external/libmaple/libmaple/usart_private.c \
	external/libmaple/libmaple/util.c \
	external/libmaple/libmaple/stm32f1/adc.c \
	external/libmaple/libmaple/stm32f1/bkp.c \
	external/libmaple/libmaple/stm32f1/dma.c \
	external/libmaple/libmaple/stm32f1/exti.c \
	external/libmaple/libmaple/stm32f1/fsmc.c \
	external/libmaple/libmaple/stm32f1/gpio.c \
	external/libmaple/libmaple/stm32f1/i2c.c \
	external/libmaple/libmaple/stm32f1/rcc.c \
	external/libmaple/libmaple/stm32f1/spi.c \
	external/libmaple/libmaple/stm32f1/timer.c \
	external/libmaple/libmaple/stm32f1/usart.c

libmaple_usb_a_SOURCES = \
	external/libmaple/libmaple/usb/stm32f1/usb.c \
	external/libmaple/libmaple/usb/stm32f1/usb_cdcacm.c \
	external/libmaple/libmaple/usb/stm32f1/usb_reg_map.c \
	external/libmaple/libmaple/usb/usb_lib/usb_core.c \
	external/libmaple/libmaple/usb/usb_lib/usb_init.c \
	external/libmaple/libmaple/usb/usb_lib/usb_mem.c \
	external/libmaple/libmaple/usb/usb_lib/usb_regs.c

LIBMAPLE = $(srcdir)/external/libmaple

tests/runner.cc: $(wildcard $(srcdir)/tests/*.h)
	$(srcdir)/external/cxxtest/bin/cxxtestgen --error-printer -o $@ $^

CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
OBJCOPY = $(CROSS_COMPILE)objcopy

FLAGS =
FLAGS += -O1 -g -Wall -fno-common -fno-section-anchors -fno-exceptions
FLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
FLAGS += -I$(srcdir)/lib -I$(srcdir) -I$(srcdir)/external/cxxtest
FLAGS += -I$(LIBMAPLE)/libmaple/include -I$(LIBMAPLE)/libmaple -I$(LIBMAPLE)/libmaple/usb/usb_lib -I$(LIBMAPLE)/libmaple/stm32f1/include -I$(LIBMAPLE)  -I$(srcdir)/platform/stm32
FLAGS += -mcpu=cortex-m3 -mthumb -save-temps=obj
FLAGS += -DMCU_STM32F103CB
FLAGS += -nostartfiles -T$(srcdir)/platform/h103/flash.ld

AM_CFLAGS = $(FLAGS) -std=gnu99
AM_CXXFLAGS = $(FLAGS) -std=gnu++0x -fno-rtti

%.bin: %.elf
	$(OBJCOPY) -Obinary $< $@

OPENOCD_CMD = nc -q 1 localhost 4444

run-openocd: blink.bin
	echo reset halt | $(OPENOCD_CMD)
	echo flash probe 0 | $(OPENOCD_CMD)
	echo stm32f1x mass_erase 0 | $(OPENOCD_CMD)
	echo flash write_bank 0 $(PWD)/$< 0 | $(OPENOCD_CMD)
	echo reset run | $(OPENOCD_CMD)

go: roverif.bin
	-[ ! -e /dev/ttyACM0 ] || python $(srcdir)/tools/reboot.py /dev/ttyACM0
	$(srcdir)/tools/opuploadtool -p $< -d 0
	$(srcdir)/tools/opuploadtool -j
