UEFI:
	$(MAKE) -C UEFI

Legacy:
	$(MAKE) -C Bootloader/ FirstStage
	$(MAKE) -C Bootloader/ SecondStage

clean:
	$(MAKE) -C Bootloader/ clean
	$(MAKE) -C UEFI/ clean
