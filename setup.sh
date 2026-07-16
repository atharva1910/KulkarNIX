#sudo apt install wget build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo nasm
#sudo apt install ovmf binutils-mingw-w64 gcc-mingw-w64 xorriso mtools
#clang install
bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
# install ovmf and mtools
sudo apt install ovmf mtools qemu-system-x86
