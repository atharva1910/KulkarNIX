UEFI:
	$(MAKE) -C UEFI

Legacy:
	$(MAKE) -C Bootloader/ FirstStage
	$(MAKE) -C Bootloader/ SecondStage

HAL:
	$(MAKE) -C HAL/

clean:
	$(MAKE) -C Bootloader/ clean
	$(MAKE) -C HAL/ clean
