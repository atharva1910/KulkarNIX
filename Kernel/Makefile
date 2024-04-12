CC = i686-elf-gcc
KERN_DIR = $(CURDIR)
CCFLAGS = -ffreestanding -fno-exceptions -O2 -g
LDFLAGS = -ffreestanding -nostdlib
LDFILE  = linker.ld
INCLUDES = -I$(ARCH_INC_DIR) -I$(PUB_INC_DIR) -I$(KERN_DIR)
LINKS = $(OUTPUT_DIR)/li686HAL.a


all: KEntry.o KPrint.o KPIC.o KIntr.o
	@echo "========== Building Kernel =========="
	$(CC) -T $(LDFILE) -o $(OUTPUT_DIR)/Kernel.bin $(LDFLAGS) $^ $(LINKS)
	objcopy --only-keep-debug $(OUTPUT_DIR)/Kernel.bin $(OUTPUT_DIR)/Kernel.sym

KEntry.o: KEntry.c
	$(CC) -c $^ $(CCFLAGS) $(INCLUDES)

KPrint.o: KPrint.c
	$(CC) -c $^ $(CCFLAGS) $(INCLUDES)

KPIC.o: KPIC.c
	$(CC) -c $^ $(CCFLAGS) $(INCLUDES)

KIntr.o: KIntr.c
	$(CC) -c $^ $(CCFLAGS) $(INCLUDES)

clean:
	-rm -f *.o *.bin *.img
