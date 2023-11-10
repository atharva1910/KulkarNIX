CC	= i686-elf-g++
ASMCC   = nasm

CCFLAGS = -ffreestanding -fno-exceptions -O2 -g
LDFLAGS = -ffreestanding -nostdlib
LDFILE  = linker.ld
INCLUDES = -I../../../../Inc/Arch/arm64 -I../../../../Inc


all: boot_main_asm.o boot_main.o x86.o
	$(CC) -T $(LDFILE) -o Boot.bin $(LDFLAGS) $^ -lgcc
	objcopy --only-keep-debug Boot.bin ../../../../Build/IIStageBootloader.sym
	objcopy -O binary Boot.bin ../../../../Build/IIStageBootloader.bin

#IIStageBootloader.o: IIStageBootloader.asm
#	$(ASMCC) -f elf32 IIStageBootloader.asm -o IIStageBootloader.o

boot_main.o: boot_main.cpp
	$(CC) -c boot_main.cpp $(CCFLAGS) $(INCLUDES)

x86.o: x86.cpp
	$(CC) -c x86.cpp $(CCFLAGS) $(INCLUDES)

boot_main_asm.o: boot_main.s
	$(CC) -c boot_main.s -o boot_main_asm.o

clean:
	rm -f *.o *.bin *.img
