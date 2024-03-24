i686:
	$(MAKE) -C i686/ Legacy
	$(MAKE) -C i686/ HAL

arm64:
	$(MAKE) -C arm64/ UEFI

clean:
	$(MAKE) -C arm64/ clean
	$(MAKE) -C i686/ clean
#	$(MAKE) -C arm64/ HAL
