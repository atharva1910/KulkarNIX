output=KulkarNIX.bin

#This is the default option
#debug:clean all
qemu-system-x86_64 -s -S -drive file=$(output),media=disk,format=raw

#release:clean all
#qemu-system-x86_64 -drive file=$(output),index=0,media=disk,format=raw

