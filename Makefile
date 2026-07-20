# --- Toolchain Configurations ---
CC      := clang++
LD      := ld.lld
QEMU    := qemu-system-x86_64

# --- Build Targets & Inputs ---
IMAGE     := nvme.img
BUILD_DIR := build
KERNEL    := $(BUILD_DIR)/Kernel.elf
EFI_BIN   := $(BUILD_DIR)/BOOTX64.EFI

# --- Compilation Flags ---
EFI_INCLUDES := -I./inc/UEFI -I./inc
EFI_CFLAGS   := $(EFI_INCLUDES) -target x86_64-pc-windows-msvc -g -ffreestanding -fshort-wchar -mno-red-zone -nostdlib -std=c++20
EFI_LDFLAGS  := -fuse-ld=lld -Wl,-entry:efi_main -Wl,-subsystem:efi_application

KERNEL_CFLAGS  := -target x86_64-unknown-none-elf -ffreestanding -g -O2 -mno-red-zone -mno-mmx -mno-sse -fno-stack-protector -nostdlib -std=c++20
KERNEL_LDFLAGS := -T kernel/linker.ld

# --- Default Goal Target ---
.PHONY: all bootloader kernel image run clean

all: bootloader kernel image

# --- Target: Bootloader Compile ---
bootloader:
	@mkdir -p $(BUILD_DIR)
	$(CC) $(EFI_CFLAGS) $(EFI_LDFLAGS) -o $(EFI_BIN) boot/main.cpp

# --- Target: Kernel Compile ---
kernel:
	@mkdir -p $(BUILD_DIR)
	$(CC) $(KERNEL_CFLAGS) -c -o $(BUILD_DIR)/main.o kernel/main.cpp
	$(LD) $(KERNEL_LDFLAGS) -o $(KERNEL) $(BUILD_DIR)/main.o

# --- Target: Create Disk & Inject Bootloader ---
image: bootloader
	dd if=/dev/zero of=$(IMAGE) bs=1M count=64
	mkfs.fat -F 32 $(IMAGE)
	mmd -i $(IMAGE) ::/EFI
	mmd -i $(IMAGE) ::/EFI/BOOT
	mcopy -i $(IMAGE) $(EFI_BIN) ::/EFI/BOOT -o
	mcopy -i $(IMAGE) $(KERNEL) :: -o

# --- Target: Emulation ---
run: all
	$(QEMU) -bios OVMF.fd -serial stdio -d cpu_reset -drive file=$(IMAGE),format=raw -display none

# --- Target: Clean Artifacts ---
clean:
	rm -rf $(BUILD_DIR) $(IMAGE)
