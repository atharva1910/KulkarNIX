CC = x86_64-w64-mingw32-gcc
LD = x86_64-w64-mingw32-gcc
CCFLAGS = -ffreestanding -O2 -g
LDFLAGS = -nostdlib
LDFILE  = linker.ld
INCLUDES = -I$(UEFI_INC_DIR) -I$(UEFI_INC_DIR)/x86_64 -I$(UEFI_INC_DIR)/protocol


all: data.o main.o
	@echo "========== Building UEFI Bootloader =========="
	$(LD) $(LDFLAGS) -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o $(OUTPUT_DIR)/BOOTx64.EFI main.o data.o

data.o:
	$(CC) $(CCFLAGS) $(INCLUDES) -c -o data.o data.c

main.o: main.c
	$(CC) $(CCFLAGS) $(INCLUDES) -c -o main.o main.c

clean:
	rm -f *.o *.bin *.img *.efi
