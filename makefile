PORT=/dev/ttyUSB0
MCU=atmega328p
TARGET=main

LDFLAGS=-Wl,-gc-sections -Wl,-relax -Wl,-Map=$(TARGET).map,--cref
#CFLAGS=-g -Wall -mcall-prologues -mmcu=$(MCU) -Os
CFLAGS=-g -Wall -mmcu=$(MCU) -O3

default: compile size

compile:
	avr-gcc $(CFLAGS) $(TARGET).c -o $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex

quick: compile size program

size:
	avr-size -C --mcu=$(MCU) $(TARGET).elf

program:
prog:
	avrdude -v -P usb -p $(MCU) -cusbasp -U flash:w:$(TARGET).hex

clean:
	rm -rf *.hex *.bin *.o *.elf doxygen_out/

asm:
	avr-objdump $(TARGET).elf -d > $(TARGET).d
