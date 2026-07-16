boot1=IStageBootloader.bin
boot2=IIStageBootloader.bin
kernel=Kernel.bin
outfile=KulkarNIX.bin

dd if=$boot1 of=$outfile bs=512 seek=0
# The second stage can be more than one sector
dd if=$boot2 of=$outfile bs=512 seek=1
# Start the kernel from the 5th sector
dd if=$kernel of=$outfile bs=512 seek=5

if [ "$1" = "Release" ]
then
	echo "Running Release"
	qemu-system-x86_64 -drive file=$outfile,index=0,media=disk,format=raw
else
	echo "Running Debug"
	qemu-system-x86_64 -s -S -drive file=$outfile,media=disk,format=raw
fi


