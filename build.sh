# Build the image using nasm
cd Bootloader
nasm -f bin Bootloader.asm -o ../Build/Bootloader.bin

# Build a floppy image using dd
cd ../Build
dd if=Bootloader.bin of=Boot.bin

# Run in qemu
qemu-system-x86_64 -fda Bootloader.bin

