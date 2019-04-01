ASMCC   = i686-elf-as
CC		= i686-elf-g++

CCFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld


all: kentry.o kernel.o 
	$(CC) -T $(LDFILE) -o ../Build/Kernel.bin $(LDFLAGS) $^ -lgcc

kentry.o:
	$(ASMCC) kentry.asm -o kentry.o

kernel.o: 
	$(CC) -c kernel.cpp $(CCFLAGS)

clean:
	rm -f *.o *.bin *.img
