#!/bin/bash
dd if=/dev/zero of=nvme.img bs=1M count=64
mkfs.fat -F 32 nvme.img
mmd -i nvme.img ::/EFI
mmd -i nvme.img ::/EFI/BOOT
mcopy -i nvme.img ../zig-out/bin/bootx64.efi ::/EFI/BOOT -o
