# Build the image using nasm
cd Bootloader
nasm -f bin Bootloader.asm -o ../Build/Bootloader.bin
# Build the second stage bootloader
nasm -f bin IIStageBootloader.asm -o ../Build/IIStageBootloader.bin

# Build a floppy image using dd
cd ../Build

# write first stage bootloader to the first sector
dd if=Bootloader.bin of=KulkarNIX.bin bs=512 seek=0
# write second stage bootloader to the second sector
dd if=IIStageBootloader.bin of=KulkarNIX.bin bs=512 seek=1

# Write kernel to the second sector
dd if=/dev/zero of=KulkarNIX.bin bs=512 seek=2 count=4

# Run in qemu
qemu-system-x86_64 -fda KulkarNIX.bin

