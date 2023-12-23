export ARCH=i686
export OUTPUT_DIR=$(CURDIR)/Bin
export PUB_INC_DIR=$(CURDIR)/Inc
export ARCH_INC_DIR=$(CURDIR)/Inc/Arch/$(ARCH)

BOOT1=$(OUTPUT_DIR)/IStageBootloader.bin
BOOT2=$(OUTPUT_DIR)/IIStageBootloader.bin
KERNEL=$(OUTPUT_DIR)/Kernel.bin
OUTFILE=$(OUTPUT_DIR)/KulkarNIX.bin

#This is the default option
debug: all MakeImage
	@echo "========== Debug Image =========="
	qemu-system-x86_64 -s -S -drive file=$(OUTFILE),index=0,media=disk,format=raw -d cpu_reset

release: all MakeImage
	@echo "========== Release Image =========="
	qemu-system-x86_64 -drive file=$(OUTFILE),index=0,media=disk,format=raw -d cpu_reset

all: Arch Kernel

Debug:
	$(MAKE) -C Common/Debug
KLibs:
	$(MAKE) -C HAL/
Arch:
	$(MAKE) -C Arch/ $(ARCH)
Kernel:
	$(MAKE) -C Kernel/ all

MakeImage:
	dd if=$(BOOT1) of=$(OUTFILE) bs=512 seek=0
	dd if=$(BOOT2) of=$(OUTFILE) bs=512 seek=1
	dd if=$(KERNEL) of=$(OUTFILE) bs=512 seek=5

clean:
	rm $(OUTPUT_DIR)/*.sym
	rm $(OUTPUT_DIR)/*.bin
	$(MAKE) -C Arch/ clean
	$(MAKE) -C Kernel/ clean
