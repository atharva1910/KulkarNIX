CC		= i686-elf-g++
LD		= i686-elf-gcc
ASMCC   = nasm

CCFLAGS = -ffreestanding -fno-exceptions -O2 -fno-rtti
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = Linker.ld

all: IIStageBootloader.o boot_main.o harddisk.o
	$(LD) -T $(LDFILE) -o Boot.bin $(LDFLAGS) $^ -lgcc
	objcopy.exe -O binary Boot.bin ../../Build/IIStageBootLoader.bin

IIStageBootloader.o: IIStageBootloader.asm
	$(ASMCC) -f elf32 IIStageBootloader.asm -o IIStageBootloader.o

boot_main.o: boot_main.cpp
	$(CC) -c $^ $(CCFLAGS)

harddisk.o: harddisk.cpp
	$(CC) -c $^ $(CCFLAGS)

clean:
	rm -f *.o *.bin *.img
