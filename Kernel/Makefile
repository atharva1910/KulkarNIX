CC = i686-elf-gcc
KERN_DIR = $(CURDIR)
CCFLAGS = -ffreestanding -fno-exceptions -O2 -g
LDFLAGS = -ffreestanding -nostdlib
LDFILE  = linker.ld
INCLUDES = -I$(ARCH_INC_DIR) -I$(PUB_INC_DIR) -I$(KERN_DIR)


all: KEntry.o KPrint.o
	@echo "========== Building Kernel =========="
	$(CC) -T $(LDFILE) -o $(OUTPUT_DIR)/Kernel.bin $(LDFLAGS) $^ -lgcc
	objcopy --only-keep-debug $(OUTPUT_DIR)/Kernel.bin $(OUTPUT_DIR)/Kernel.sym

KEntry.o: KEntry.c
	$(CC) -c KEntry.c $(CCFLAGS) $(INCLUDES)

KPrint.o: KPrint.c
	$(CC) -c KPrint.c $(CCFLAGS) $(INCLUDES)

clean:
	rm -f *.o *.bin *.img
