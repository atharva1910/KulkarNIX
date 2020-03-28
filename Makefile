output = Build/KulkarNIX.bin

#This is the default option
debug:clean Debug all
	qemu-system-x86_64 -s -S -drive file=$(output),media=disk,format=raw

release:clean all
	qemu-system-x86_64 -drive file=$(output),index=0,media=disk,format=raw

build:clean all

all: KLibs FirstStage SecondStage kernel WriteImage

Debug:
	$(MAKE) -C Common/Debug
KLibs:
	$(MAKE) -C HAL/
FirstStage:
	$(MAKE) -C Bootloader/FirstStage
SecondStage:
	$(MAKE) -C Bootloader/SecondStage
kernel:
	$(MAKE) -C Kernel/ all

WriteImage:
	$(MAKE) -C Build/ 


clean:
	$(MAKE) -C Build/ clean
	$(MAKE) -C Kernel/ clean
	$(MAKE) -C Bootloader/FirstStage clean
	$(MAKE) -C Bootloader/SecondStage clean
	$(MAKE) -C HAL/ clean
	$(MAKE) -C Common/Debug clean


