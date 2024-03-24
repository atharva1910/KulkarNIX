AR = i686-elf-ar
CC = i686-elf-gcc
CCFLAGS = -ffreestanding -fno-exceptions -O2 -g
LDFLAGS = -ffreestanding -nostdlib
LDFILE  = linker.ld
INCLUDES = -I$(ARCH_INC_DIR) -I$(PUB_INC_DIR)


all: x86.o
	@echo "========== Building $(ARCH) HAL =========="
	$(AR) rcs $(OUTPUT_DIR)/li686HAL.a $^
	objcopy --only-keep-debug li686HAL.a $(OUTPUT_DIR)/li686HAL.sym

x86.o: x86.c
	$(CC) -c x86.c $(CCFLAGS) $(INCLUDES)

clean:
	-rm -f *.o *.bin *.img *.a
