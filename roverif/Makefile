APP = roverif
DEVICE     = atmega168
CLOCK      = 8000000
#PROGRAMMER = -c signalyzer-lite
PROGRAMMER = -c arduino -b 19200 -P $(PROGRAMMER_PORT)
PROGRAMMER_PORT = /dev/ttyO4

SRC = $(wildcard *.cc)
SRC += $(wildcard ../lib/*.cc)
OBJ = $(SRC:%.cc=%.o)
LIB = $(APP).a
FUSES = -U lfuse:w:0xe2:m -U hfuse:w:0xde:m -U efuse:w:0xfe:m

CROSS_COMPILE = avr-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
CFLAGS = -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -Wall
CFLAGS += -I../lib -I../external/avr-stl-stubs/include
CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections -pipe
#CFLAGS += -save-temps
CXXFLAGS = $(CFLAGS) -std=gnu++11
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
RESET_PIN = /sys/class/gpio/gpio9/value

all:	$(APP).hex

reset:
	(echo 1 | sudo tee $(RESET_PIN)) && \
	(echo 0 | sudo tee $(RESET_PIN)) && \
	(echo 1 | sudo tee $(RESET_PIN))

flash:	all reset
	$(AVRDUDE) -U flash:w:$(APP).hex:i

usb-flash: $(APP).hex
	sudo $(HOME)/bin/micronucleus --run $<

fuse:
	$(AVRDUDE) $(FUSES)

%.elf: start.o $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $^
	$(CROSS_COMPILE)size -d $@

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

clean:
	rm -f $(APP).hex $(APP).elf $(OBJ) *~
