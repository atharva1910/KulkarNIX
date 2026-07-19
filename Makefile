# --- Toolchain Configurations ---
CC      := clang
LD      := ld.lld
QEMU    := qemu-system-x86_64.exe

# --- Directory Structures ---
BUILD_DIR := build
EFI_DIR   := $(DISK_DIR)/EFI/BOOT

# --- Compilation Flags ---
# UEFI Bootloader Flags
EFI_INCLUDES := -I./inc/UEFI -I./inc
EFI_CFLAGS   := $(EFI_INCLUDES) -target x86_64-pc-windows-msvc -g -ffreestanding -fshort-wchar -mno-red-zone -nostdlib
EFI_LDFLAGS  := -fuse-ld=lld -Wl,-entry:efi_main -Wl,-subsystem:efi_application

# Bare-Metal Kernel Flags
KERNEL_CFLAGS  := -target x86_64-unknown-none-elf -ffreestanding -g -O2 -mno-red-zone -mno-mmx -mno-sse -fno-stack-protector -nostdlib
KERNEL_LDFLAGS := -T kernel/linker.ld

# --- Default Goal Target ---
.PHONY: all bootloader kernel run clean

all: bootloader kernel

# --- Target: Bootloader ---
bootloader:
	@mkdir -p $(EFI_DIR)
	$(CC) $(EFI_CFLAGS) $(EFI_LDFLAGS) -o $(BUILD_DIR)/BOOTX64.EFI boot/main.c

# --- Target: Kernel ---
kernel:
	@mkdir -p $(BUILD_DIR)
	$(CC) $(KERNEL_CFLAGS) -c -o $(BUILD_DIR)/main.o kernel/main.c
	$(LD) $(KERNEL_LDFLAGS) -o $(BUILD_DIR)/Kernel.elf $(BUILD_DIR)/main.o

# --- Target: Emulation ---
run: all
	$(QEMU) -bios OVMF.fd -serial stdio -d cpu_reset -drive file=fat:rw:$(DISK_DIR),format=raw -display none

# --- Target: Clean Artifacts ---
clean:
	@if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	@if exist $(DISK_DIR) rmdir /s /q $(DISK_DIR)
