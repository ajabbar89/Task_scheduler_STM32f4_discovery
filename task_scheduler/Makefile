include sources.mk


MACH = cortex-m4

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
CFLAGS = -Wall -c -mcpu=$(MACH) -mthumb -save-temps -std=gnu11 -O0 
LDFLAGS = -mcpu=$(MACH) -mthumb -T linker_script.ld -nostdlib -Wl,-Map=final.map

OBJS=$(SRCS:.c=.o)
%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c $<

all: build

build: $(OBJS)
	$(CC) *.o $(LDFLAGS) -o final.elf

clean:
	rm -f *.o *.i *.s *.elf

load:
	openocd -f stm32f4discovery.cfg
