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
	qemu-system-x86_64 -s -S -bios /usr/share/ovmf/OVMF.fd -net none -drive format=raw,unit=0,file=$(OUTPUT_DIR)/UEFI.img

release: UEFI Kernel MakeUEFI
	@echo "========== UEFI Release Image =========="
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -net none -drive format=raw,unit=0,file=$(OUTPUT_DIR)/UEFI.img

UEFI:
	$(MAKE) -C Arch/ $(BOOT)

MakeUEFI: MakeFS MakeMount MakeCP MakeCleanup

MakeFS:
ifneq ("$(wildcard $(OUTPUT_DIR)/UEFI.img)","")
	@echo "========== FS image exists, not recreating image =========="
	sudo losetup --offset 1048576 --sizelimit 46934528 /dev/loop42 $(OUTPUT_DIR)/UEFI.img
	sudo mkdosfs -F 32 /dev/loop42
else
	dd if=/dev/zero of=$(OUTPUT_DIR)/UEFI.img bs=512 count=93750
	gdisk $(OUTPUT_DIR)/UEFI.img
	sudo losetup --offset 1048576 --sizelimit 46934528 /dev/loop42 $(OUTPUT_DIR)/UEFI.img
	sudo mkdosfs -F 32 /dev/loop42
endif

MakeMount:
	rm -rf $(OUTPUT_DIR)/FakeMount
	mkdir $(OUTPUT_DIR)/FakeMount
	sudo mount /dev/loop42 $(OUTPUT_DIR)/FakeMount
	sudo mkdir -p $(OUTPUT_DIR)/FakeMount/EFI/BOOT

MakeCP:
	sudo cp $(OUTPUT_DIR)/BOOTX64.EFI $(OUTPUT_DIR)/FakeMount/EFI/BOOT/
	sudo cp $(OUTPUT_DIR)/Kernel.bin $(OUTPUT_DIR)/FakeMount/

MakeCleanup:
	sudo umount $(OUTPUT_DIR)/FakeMount
	rm -rf $(OUTPUT_DIR)/FakeMount
	sudo losetup -d /dev/loop42

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
	-rm $(OUTPUT_DIR)/*.sym
	-rm $(OUTPUT_DIR)/*.bin
	-rm $(OUTPUT_DIR)/*.EFI
	-rm $(OUTPUT_DIR)/*.img
	$(MAKE) -C Arch/ clean
	$(MAKE) -C Kernel/ clean
