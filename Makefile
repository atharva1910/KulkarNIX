export OUTPUT_DIR=$(CURDIR)/Bin
export PUB_INC_DIR=$(CURDIR)/Inc
export ARCH_INC_DIR=$(CURDIR)/Inc/Arch/arm64
boot1=IStageBootloader.bin
boot2=IIStageBootloader.bin
kernel=Kernel.bin
outfile=KulkarNIX.bin

#This is the default option
debug: clean all
	qemu-system-x86_64 -s -S -drive file=$(output),media=disk,format=raw

release: clean all
	qemu-system-x86_64 -drive file=$(output),index=0,media=disk,format=raw

#all: Debug KLibs FirstStage SecondStage kernel WriteImage
all: Arch Kernel

Debug:
	$(MAKE) -C Common/Debug
KLibs:
	$(MAKE) -C HAL/
Arch:
	$(MAKE) -C Arch/ arm64
Kernel:
	$(MAKE) -C Kernel/ all

clean:
	$(MAKE) -C Arch/ clean
	$(MAKE) -C Kernel/ clean
