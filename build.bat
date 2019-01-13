@echo off

:: Build the image using nasm
nasm -f bin Bootloader.asm -o Bootloader.bin

:: Run in qemu
qemu-system-x86_64 -fda Bootloader.bin

:: pause
::pause
