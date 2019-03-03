ASMCC   = nasm
CC	= i686-elf-gcc

CCFLAGS = -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld


all: kentry.o kernel.o 
	$(CC) -T $(LDFILE) -o ../Build/Kernel.bin $(LDFLAGS) $^ -lgcc

kentry.o:
	$(ASMCC) -f elf32 kentry.asm -o kentry.o

kernel.o: 
	$(CC) -c kernel.c $(CCFLAGS)

clean:
	rm -f *.o *.bin *.img
