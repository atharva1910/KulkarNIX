@echo off

:: Build the image using nasm
nasm -f bin Bootloader\Bootloader.asm -o Build\Bootloader.bin

:: Run in qemu
qemu-system-x86_64 -fda Build\Bootloader.bin

:: pause
::pause
