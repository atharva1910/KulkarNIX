@echo off

:: Build the image using nasm
cd Bootloader
nasm -f bin Bootloader.asm -o ..\Build\Bootloader.bin

:: Run in qemu
cd ..\Build
qemu-system-x86_64 -fda Bootloader.bin
::qemu-system-x86_64 -drive format=raw,file=Bootloader.bin,index=0,if=floppy

:: pause
pause