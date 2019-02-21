boot1 = IStageBootloader.bin
boot2 = IIStageBootloader.bin
kernel = Kernel.bin
outfile = KulkarNIX.bin

write: 
	dd if=$(boot1) of=$(outfile) bs=512 seek=0
	# The second stage can be more than one sector
	dd if=$(boot2) of=$(outfile) bs=512 seek=1
	# Start the kernel from the 5th sector
	dd if=$(boot1) of=$(outfile) bs=512 seek=5

clean:
	rm -f *.bin *.img *.sym

