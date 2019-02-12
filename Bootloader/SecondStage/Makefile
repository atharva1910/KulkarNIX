CC		= i686-elf-gcc
ASMCC   = nasm

CCFLAGS = -ffreestanding -fno-exceptions -O2 -g
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld

all: IIStageBootloader.o boot_main.o
	$(CC) -T $(LDFILE) -o Boot.bin $(LDFLAGS) $^ -lgcc
	objcopy --only-keep-debug Boot.bin ../../Build/IIStageBootloader.sym
	objcopy -O binary Boot.bin ../../Build/IIStageBootloader.bin

IIStageBootloader.o: IIStageBootloader.asm
	$(ASMCC) -f elf32 IIStageBootloader.asm -o IIStageBootloader.o

boot_main.o: 
	$(CC) -c boot_main.c $(CCFLAGS)

clean:
	rm -f *.o *.bin *.img
