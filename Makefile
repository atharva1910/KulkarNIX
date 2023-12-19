export OUTPUT_DIR=$(CURDIR)/Bin
export PUB_INC_DIR=$(CURDIR)/Inc
export ARCH_INC_DIR=$(CURDIR)/Inc/Arch/arm64

boot1=$(OUTPUT_DIR)/IStageBootloader.bin
boot2=$(OUTPUT_DIR)/IIStageBootloader.bin
kernel=$(OUTPUT_DIR)/Kernel.bin
outfile=$(OUTPUT_DIR)/KulkarNIX.bin

#This is the default option
debug: clean all MakeImage
	@echo "========== Debug Image =========="
	qemu-system-x86_64 -s -S -drive file=$(outfile),index=0,media=disk,format=raw

release: clean all MakeImage
	@echo "========== Release Image =========="
	qemu-system-x86_64 -drive file=$(outfile),index=0,media=disk,format=raw

all: Arch Kernel

Debug:
	$(MAKE) -C Common/Debug
KLibs:
	$(MAKE) -C HAL/
Arch:
	$(MAKE) -C Arch/ arm64
Kernel:
	$(MAKE) -C Kernel/ all

MakeImage:
	dd if=$(boot1) of=$(outfile) bs=512 seek=0
	dd if=$(boot2) of=$(outfile) bs=512 seek=1
	dd if=$(kernel) of=$(outfile) bs=512 seek=5

clean:
	rm $(OUTPUT_DIR)/*.sym
	rm $(OUTPUT_DIR)/*.bin
	$(MAKE) -C Arch/ clean
	$(MAKE) -C Kernel/ clean
