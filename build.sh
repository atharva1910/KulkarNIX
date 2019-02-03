# Build the first stage bootloader
cd Bootloader/FirstStage
nasm -f bin Bootloader.asm -o ../../Build/IStageBootloader.bin
# Build the second stage bootloader
cd ../SecondStage
nasm -f elf32 IIStageBootloader.asm -o IIStageBootloader.o
i686-elf-g++ -c boot_main.cpp -o boot_main.o -ffreestanding -fno-exceptions -O2 -fno-rtti
i686-elf-gcc -T Linker.ld  -o 2Boot.bin -ffreestanding -nostdlib IIStageBootloader.o boot_main.o -lgcc
objcopy.exe -O binary 2Boot.bin ../../Build/IIStageBootloader.bin
rm -f *.o *.bin *.img

# Build a floppy image using dd
cd ../../Build

# write first stage bootloader to the first sector
dd if=IStageBootloader.bin of=KulkarNIX.bin bs=512 seek=0
# write second stage bootloader to the second sector
dd if=IIStageBootloader.bin of=KulkarNIX.bin bs=512 seek=1

# fill with zeros just because
dd if=/dev/zero of=KulkarNIX.bin bs=512 seek=2 count=4

# Run in qemu
qemu-system-x86_64 -fda KulkarNIX.bin

