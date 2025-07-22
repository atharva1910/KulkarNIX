#!/bin/bash
mcopy -i nvme.img ../zig-out/bin/bootx64.efi ::/EFI/BOOT -o
mcopy -i nvme.img ../zig-out/bin/Tramp.elf ::/ -o
mcopy -i nvme.img ../zig-out/bin/Kernel.elf ::/ -o
qemu-system-x86_64 -bios /usr/share/OVMF/OVMF_CODE.fd -drive format=raw,file=nvme.img -serial stdio -d cpu_reset
