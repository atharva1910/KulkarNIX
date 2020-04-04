ASMCC   = i686-elf-as
CC		= i686-elf-g++

CCFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -g
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld
INCLUDES = -I../Common -I../Include
LINCLUDES = ../Lib/


all: kentry.o kernel.o
	$(CC) -T $(LDFILE) -o ../Build/Kernel.bin $(LDFLAGS) $^ -lgcc $(LINCLUDES)/HAL/KLibHAL.a  $(LINCLUDES)/KDebugPrints.o
	objcopy --only-keep-debug ../Build/Kernel.bin ../Build/Kernel.sym

kentry.o:
	$(ASMCC) kentry.asm -o kentry.o

kernel.o: 
	$(CC) -c kernel.cpp $(CCFLAGS) $(INCLUDES)

clean:
	rm -f *.o *.bin *.img
