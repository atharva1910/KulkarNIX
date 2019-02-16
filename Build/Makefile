boot1 = IStageBootloader.bin
boot2 = IIStageBootloader.bin
outfile = KulkarNIX.bin

debug: write
	qemu-system-x86_64 -s -S -drive file=KulkarNIX.bin,media=disk,format=raw

release: write
	qemu-system-x86_64 -drive file=KulkarNIX.bin,media=disk,format=raw

write: 
	dd if=$(boot1) of=$(outfile) bs=512 seek=0
	dd if=$(boot2) of=$(outfile) bs=512 seek=1
	dd if=$(boot1) of=$(outfile) bs=512 seek=4
	dd if=/dev/zero of=$(outfile) bs=512 seek=3 count=4

clean:
	rm -f *.bin *.img *.sym

