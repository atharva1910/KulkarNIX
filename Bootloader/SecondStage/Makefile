CC	= i686-elf-g++
ASMCC   = nasm

CCFLAGS = -ffreestanding -fno-exceptions -O2 -g
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld
INCLUDES = -I../../Common -I../../Include/HAL
LINCLUDES = ../../Lib/HAL


all: IIStageBootloader.o boot_main.o
	$(CC) -T $(LDFILE) -o Boot.bin $(LDFLAGS) $^ -lgcc $(LINCLUDES)/libx86.a
	objcopy --only-keep-debug Boot.bin ../../Build/IIStageBootloader.sym
	objcopy -O binary Boot.bin ../../Build/IIStageBootloader.bin

IIStageBootloader.o: IIStageBootloader.asm
	$(ASMCC) -f elf32 IIStageBootloader.asm -o IIStageBootloader.o

boot_main.o: 
	$(CC) -c boot_main.cpp $(CCFLAGS) $(INCLUDES)

clean:
	rm -f *.o *.bin *.img
