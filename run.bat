clang -I.\inc\UEFI -target x86_64-pc-windows-msvc -ffreestanding -fshort-wchar -mno-red-zone -nostdlib -fuse-ld=lld -Wl,-entry:efi_main -Wl,-subsystem:efi_application -o BOOTX64.EFI arch\amd64\main.c

move BOOTX64.EFI disk\EFI\BOOT

"C:\\Program Files\\qemu\\qemu-system-x86_64.exe" -bios OVMF.fd -serial stdio -d cpu_reset -drive file=fat:rw:disk,format=raw -display none
