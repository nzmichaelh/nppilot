APP = roverif
DEVICE     = atmega168
CLOCK      = 12000000
#PROGRAMMER = -c signalyzer-lite
PROGRAMMER = -c arduino -b 57600 -P $(PROGRAMMER_PORT)
PROGRAMMER_PORT = /dev/ttyO4
REMOTE = blue

I2CDEVLIB = ../external/i2cdevlib/Arduino
SRC = $(wildcard *.cc)
SRC += $(wildcard ../lib/*.cc)
SRC += $(I2CDEVLIB)/MPU6050/MPU6050.cpp

OBJ = $(SRC:%.cc=%.o)
OBJ := $(OBJ:%.cpp=%.o)
LIB = $(APP).a

CROSS_COMPILE = avr-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
CFLAGS = -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -Wall
CFLAGS += -I../lib -I../external/avr-stl-stubs/include -I.
CFLAGS += -I$(I2CDEVLIB) -I$(I2CDEVLIB)/I2Cdev
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections -pipe
#CFLAGS += -save-temps
CXXFLAGS = $(CFLAGS) -std=gnu++11
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
RESET_PIN = /sys/class/gpio/gpio9/value

all:	$(APP).hex

reset:
	bash -c "echo -e '1\n0\n1'" > $(RESET_PIN)

sub-flash:
	$(AVRDUDE) -U flash:w:$(APP).hex:i

flash: all reset

usb-flash: $(APP).hex
	sudo $(HOME)/bin/micronucleus --run $<

remote-flash: $(APP).hex
	scp $< Makefile $(REMOTE):/tmp
	ssh $(REMOTE) make -C /tmp reset sub-flash

ping:
	$(AVRDUDE)

style: all
	cpplint --filter=-legal/copyright,-whitespace/semicolon,-build/include,-build/header_guard *.cc *.h ../lib/*.cc ../lib/*.h

main.cc: version.h

version.h: version.h.tmp
	cmp -s $@ $< || mv -f $< $@

version.h.tmp:
	echo static const char* const version = \"$(shell git describe --tags --dirty=x --always)\"\; > $@

%.elf: $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
	$(CROSS_COMPILE)size -d $@

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

clean:
	rm -f $(OBJ) *.hex *.elf version.h *.o *~
