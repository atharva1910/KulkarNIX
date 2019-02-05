boot1 = IStageBootloader.bin
boot2 = IIStageBootloader.bin
outfile = KulkarNIX.bin

run: 
	dd if=$(boot1) of=$(outfile) bs=512 seek=0
	dd if=$(boot2) of=$(outfile) bs=512 seek=1
	dd if=/dev/zero of=$(outfile) bs=512 seek=2 count=4
	qemu-system-x86_64 -drive file=KulkarNIX.bin,media=disk,format=raw

clean:
	rm -f *.bin *.img

