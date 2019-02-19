output = Build/KulkarNIX.bin

#By default debug is built
debug:clean all
	qemu-system-x86_64 -s -S -drive file=$(output),media=disk,format=raw

release:clean all
	qemu-system-x86_64 -drive file=$(output),index=0,media=disk,format=raw

all: FirstStage SecondStage WriteImage

FirstStage:
	$(MAKE) -C Bootloader/FirstStage
SecondStage:
	$(MAKE) -C Bootloader/SecondStage
WriteImage:
	$(MAKE) -C Build/ 

run:

clean:
	$(MAKE) -C Build/ clean
	$(MAKE) -C Bootloader/FirstStage clean
	$(MAKE) -C Bootloader/SecondStage clean

