CC		= i686-elf-g++
LD		= i686-elf-gcc
ASMCC   = nasm

CCFLAGS = -ffreestanding -fno-exceptions -O2 -fno-rtti
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld

all: IIStageBootloader.o boot_main.o Harddisk.o Port.o
	$(LD) -T $(LDFILE) -o Boot.bin $(LDFLAGS) $^ -lgcc
	objcopy -O binary Boot.bin ../../Build/IIStageBootloader.bin

IIStageBootloader.o: IIStageBootloader.asm
	$(ASMCC) -f elf32 IIStageBootloader.asm -o IIStageBootloader.o

boot_main.o: 
	$(CC) -c boot_main.cpp $(CCFLAGS)

Harddisk.o: 
	$(CC) -c Harddisk.cpp $(CCFLAGS)

Port.o:
	$(CC) -c Port.cpp $(CCFLAGS)

clean:
	rm -f *.o *.bin *.img
