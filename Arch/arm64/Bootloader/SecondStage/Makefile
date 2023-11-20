CC = i686-elf-g++

CCFLAGS = -ffreestanding -fno-exceptions -O2 -g
LDFLAGS = -ffreestanding -nostdlib
LDFILE  = linker.ld
INCLUDES = -I$(ARCH_INC_DIR) -I$(PUB_INC_DIR)


all: SecondStage.o x86.o SecondStageHelper.o
	@echo "Building SecondStage Bootloader"
	$(CC) -T $(LDFILE) -o Boot.bin $(LDFLAGS) $^ -lgcc
	objcopy --only-keep-debug Boot.bin $(OUTPUT_DIR)/IIStageBootloader.sym
	objcopy -O binary Boot.bin $(OUTPUT_DIR)/IIStageBootloader.bin

SecondStage.o: SecondStage.cpp
	$(CC) -c SecondStage.cpp $(CCFLAGS) $(INCLUDES)

x86.o: x86.cpp
	$(CC) -c x86.cpp $(CCFLAGS) $(INCLUDES)

SecondStageHelper.o: SecondStageHelper.s
	$(CC) -c SecondStageHelper.s $(CCFLAGS) $(INCLUDES)

clean:
	rm -f *.o *.bin *.img
