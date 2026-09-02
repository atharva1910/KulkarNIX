# --- Toolchain Configurations ---
CC    := clang++
LD    := ld.lld
QEMU  := qemu-system-x86_64

# --- Build Targets & Inputs ---
IMAGE     := nvme.img
BUILD_DIR := build
KERNEL_BIN:= $(BUILD_DIR)/Kernel.elf
EFI_BIN   := $(BUILD_DIR)/BOOTX64.EFI

.PHONY: all bootloader kernel image run clean

all: bootloader kernel image

# --- Target: Bootloader Compile ---
bootloader:
	@mkdir -p $(BUILD_DIR)
	cd boot && cargo build --release
	cp boot/target/x86_64-unknown-uefi/release/boot.efi $(EFI_BIN)

# --- Target: Kernel Compile ---
kernel:
	cd kernel && cargo build --release
	cp kernel/target/x86_64-unknown-none/release/kernel $(KERNEL_BIN)

# --- Target: Create Disk & Inject Bootloader ---
image: bootloader kernel
	dd if=/dev/zero of=$(IMAGE) bs=1M count=64
	mkfs.fat -F 32 $(IMAGE)
	mmd -i $(IMAGE) ::/EFI
	mmd -i $(IMAGE) ::/EFI/BOOT
	mcopy -i $(IMAGE) $(EFI_BIN) ::/EFI/BOOT
	mcopy -i $(IMAGE) $(KERNEL_BIN) ::

# --- Target: Emulation ---
run: all
	$(QEMU) -bios OVMF.fd -serial stdio -d cpu_reset -drive file=$(IMAGE),format=raw  -m 4G

# --- Target: Clean Artifacts ---
clean:
	rm -rf boot/target
	rm -rf kernel/target
	rm -rf common/target
