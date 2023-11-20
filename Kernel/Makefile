CC = x86_64-elf-g++

CCFLAGS = -ffreestanding -fno-exceptions -O2 -g
LDFLAGS = -ffreestanding -nostdlib
LDFILE  = linker.ld
INCLUDES = -I$(ARCH_INC_DIR) -I$(PUB_INC_DIR)


all: kernel.o
	@echo "Building Kernel"
	$(CC) -T $(LDFILE) -o $(OUTPUT_DIR)/Kernel.bin $(LDFLAGS) $^ -lgcc

kernel.o: kernel.cpp
	$(CC) -c kernel.cpp $(CCFLAGS) $(INCLUDES)

clean:
	rm -f *.o *.bin *.img
