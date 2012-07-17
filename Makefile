#
# Makefile for some simple CopterControl programs.
#
# Michael Hope <michaelh@juju.net.nz>
#

CROSS_COMPILE = arm-none-eabi-

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
OBJCOPY = $(CROSS_COMPILE)objcopy

FLAGS = -mcpu=cortex-m3 -mthumb -O -g -Wall -fno-common
#FLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
FLAGS += -Ilibmaple/libmaple/include -Ilibmaple/libmaple/stm32f1/include -Ilibmaple -Ilib -I.
FLAGS += -DMCU_STM32F103CB
FLAGS += -fno-exceptions -nostartfiles -save-temps=obj

CXXFLAGS = $(FLAGS)
CXXFLAGS += -std=gnu++0x

SRC = $(wildcard src/*.cc lib/*.cc platform/stm32/*.cc)
OBJ = $(SRC:.cc=.o)

all: nppilot.elf

nppilot.elf: $(OBJ) libmaple.a libmaple-usb.a
	$(CXX) $(CXXFLAGS) -T flash.ld -o $@ $^

%.bin: %.elf
	$(OBJCOPY) -Obinary $< $@

LIBMAPLE_SRC = $(wildcard libmaple/libmaple/*.c) $(wildcard libmaple/libmaple/stm32f1/*.c)
LIBMAPLE_OBJ = $(LIBMAPLE_SRC:.c=.o)

libmaple.a: $(LIBMAPLE_OBJ)
	ar r $@ $^

LIBMAPLE_USB_SRC = $(wildcard libmaple/libmaple/usb/stm32f1/*.c libmaple/libmaple/usb/usb_lib/*.c)
LIBMAPLE_USB_OBJ = $(LIBMAPLE_USB_SRC:.c=.o)

libmaple-usb.a: $(LIBMAPLE_USB_OBJ)
	ar r $@ $^

go: nppilot.bin
	-[ ! -e /dev/ttyACM0 ] || python tools/reboot.py /dev/ttyACM0
	tools/opuploadtool -p $< -d 0
	tools/opuploadtool -j

clean:
	rm -f $(OBJ) $(OBJ:.o=.ii) $(OBJ:.o=.s) *.elf *.bin *.a
