export BOOT=uefi-i686
#export BOOT=leg-i686
export ARCH=i686
export OUTPUT_DIR=$(CURDIR)/Bin
export PUB_INC_DIR=$(CURDIR)/Inc
export ARCH_INC_DIR=$(CURDIR)/Inc/Arch/$(ARCH)
export UEFI_INC_DIR=$(CURDIR)/Inc/UEFI
KERNEL=$(OUTPUT_DIR)/Kernel.bin
OUTFILE=$(OUTPUT_DIR)/KulkarNIX.bin

# Section : UEFI
debug: UEFI Kernel MakeUEFI
	@echo "========== UEFI Debug Image =========="
	qemu-system-x86_64 -s -S -L /usr/share/ovmf -bios /usr/share/ovmf/OVMF.fd

release: UEFI Kernel MakeUEFI
	@echo "========== UEFI Release Image =========="
	qemu-system-x86_64 -L /usr/share/ovmf -bios /usr/share/ovmf/OVMF.fd
	qemu-system-x86_64 -drive file=$(OUTFILE),index=0,media=disk,format=raw -d cpu_reset

UEFI:
	$(MAKE) -C Arch/ $(BOOT)

MakeUEFI:
	@echo "========== Making UEFI Image =========="

# Section : Legacy Bootloader
BOOT1=$(OUTPUT_DIR)/IStageBootloader.bin
BOOT2=$(OUTPUT_DIR)/IIStageBootloader.bin

ldebug: BIOS Kernel MakeLegacy
	@echo "========== Debug Image =========="
	qemu-system-x86_64 -s -S -drive file=$(OUTFILE),index=0,media=disk,format=raw -d cpu_reset

lrelease: BIOS Kernel MakeLegacy
	@echo "========== Release Image =========="
	qemu-system-x86_64 -drive file=$(OUTFILE),index=0,media=disk,format=raw -d cpu_reset

BIOS:
	$(MAKE) -C Arch/

MakeLegacy:
	dd if=$(BOOT1) of=$(OUTFILE) bs=512 seek=0
	dd if=$(BOOT2) of=$(OUTFILE) bs=512 seek=1
	dd if=$(KERNEL) of=$(OUTFILE) bs=512 seek=5

# Section Kernel, common for both legacy and UEFI bootloaders
Debug:
	$(MAKE) -C Common/Debug
KLibs:
	$(MAKE) -C HAL/
Kernel:
	$(MAKE) -C Kernel/ all

# Section : Clean
clean:
	rm $(OUTPUT_DIR)/*.sym
	rm $(OUTPUT_DIR)/*.bin
	rm $(OUTPUT_DIR)/*.efi
	$(MAKE) -C Arch/ clean
	$(MAKE) -C Kernel/ clean
