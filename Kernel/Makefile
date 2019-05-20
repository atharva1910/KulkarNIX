ASMCC   = i686-elf-as
CC		= i686-elf-g++

CCFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld
INCLUDES = -I../Common


all: kentry.o kernel.o phyMM.o
	$(CC) -T $(LDFILE) -o ../Build/Kernel.bin $(LDFLAGS) $^ -lgcc

kentry.o:
	$(ASMCC) kentry.asm -o kentry.o

kernel.o: 
	$(CC) -c kernel.cpp $(CCFLAGS) $(INCLUDES)

phyMM.o: 
	$(CC) -c phyMM.cpp $(CCFLAGS) $(INCLUDES)

clean:
	rm -f *.o *.bin *.img
