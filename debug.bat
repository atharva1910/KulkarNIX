del disk\EFI\BOOT\BOOTX64.EFI
del disk\Kernel.bin

clang -I.\inc\UEFI -I.\inc -target x86_64-pc-windows-msvc -g -ffreestanding -fshort-wchar -mno-red-zone -nostdlib -fuse-ld=lld -Wl,-entry:efi_main -Wl,-subsystem:efi_application -o BOOTX64.EFI boot\main.c

clang -target x86_64-unknown-none-elf -ffreestanding -g -O2 -mno-red-zone -mno-mmx -mno-sse -fno-stack-protector -nostdlib -c -o build\main.o kernel\main.c

ld.lld -T kernel\linker.ld -o build\Kernel.bin build\main.o

move BOOTX64.EFI disk\EFI\BOOT
move BOOTX64.pdb sym
copy build\Kernel.bin disk

"C:\\Program Files\\qemu\\qemu-system-x86_64.exe" -s -S -bios OVMF.fd -serial stdio -d cpu_reset -drive file=fat:rw:disk,format=raw -display none

del disk\EFI\BOOT\BOOTX64.EFI
del disk\Kernel.bin
