#
# Makefile for some simple CopterControl programs.
#
# Michael Hope <michaelh@juju.net.nz>
#

CROSS_COMPILE = arm-none-eabi-

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
OBJCOPY = $(CROSS_COMPILE)objcopy

LIBMAPLE = external/libmaple

FLAGS = -mcpu=cortex-m3 -mthumb -Os -g -Wall -fno-common -fno-section-anchors
FLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
FLAGS += -I$(LIBMAPLE)/libmaple/include -I$(LIBMAPLE)/libmaple -I$(LIBMAPLE)/libmaple/usb/usb_lib -I$(LIBMAPLE)/libmaple/stm32f1/include -I$(LIBMAPLE) -Ilib -I.
FLAGS += -DMCU_STM32F103CB
FLAGS += -fno-exceptions -nostartfiles -save-temps=obj

CFLAGS = $(FLAGS)

CXXFLAGS = $(FLAGS)
CXXFLAGS += -std=gnu++0x

SRC = $(wildcard src/*.cc lib/*.cc platform/stm32/*.cc)
OBJ = $(SRC:.cc=.o)

all: nppilot.elf

nppilot.elf: $(OBJ) libmaple.a libmaple-usb.a
	$(CXX) $(CXXFLAGS) -T flash.ld -o $@ $^

%.bin: %.elf
	$(OBJCOPY) -Obinary $< $@

LIBMAPLE_SRC = $(wildcard $(LIBMAPLE)/libmaple/*.c) $(wildcard $(LIBMAPLE)/libmaple/stm32f1/*.c)
LIBMAPLE_OBJ = $(LIBMAPLE_SRC:.c=.o)

libmaple.a: $(LIBMAPLE_OBJ)
	ar r $@ $^

LIBMAPLE_USB_SRC = $(wildcard $(LIBMAPLE)/libmaple/usb/stm32f1/*.c $(LIBMAPLE)/libmaple/usb/usb_lib/*.c)
LIBMAPLE_USB_OBJ = $(LIBMAPLE_USB_SRC:.c=.o)

libmaple-usb.a: $(LIBMAPLE_USB_OBJ)
	ar r $@ $^

go: nppilot.bin
	-[ ! -e /dev/ttyACM0 ] || python tools/reboot.py /dev/ttyACM0
	tools/opuploadtool -p $< -d 0
	tools/opuploadtool -j

ALL_OBJ = $(OBJ) $(LIBMAPLE_USB_OBJ) $(LIBMAPLE_OBJ)

clean:
	rm -f $(ALL_OBJ) $(ALL_OBJ:.o=.ii) $(ALL_OBJ:.o=.s) *.elf *.bin *.a
