# Install wget
sudo apt install wget build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo

# Create working Directory
mkdir KSetup
cd KSetup

# Download gcc and wget
wget https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-10.2.0/gcc-10.2.0.tar.gz

# Install dependencies
sudo apt install base-devel
sudo apt install gmp libmpc mpfr

# UEFI
sudo apt install ovmf binutils-mingw-w64 gcc-mingw-w64 xorriso mtools

# Variables for cross compiling
export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# Install binuitls for x86_64
tar -xvf binutils-2.34.tar.xz
mkdir build-binutils
cd build-binutils
../binutils-2.34/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
sudo make install
cd ..

# Install gcc for x86_64
which -- $TARGET-as || echo $TARGET-as is not in the PATH
tar xvzf gcc-10.2.0.tar.gz
mkdir build-gcc
cd build-gcc
../gcc-10.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
cd ..

export TARGET=i686-elf
# Install binutils for i686
rm -rf build-binutils
mkdir build-binutils
cd build-binutils
../binutils-2.34/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
sudo make install
cd ..

# Install gcc for i686
rm -rf build-gcc
mkdir build-gcc
cd build-gcc
../gcc-10.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
cd ..


 Install nasm and qemu required for running the OS
sudo apt install qemu nasm

# Download and install cmake-18.2
wget https://github.com/Kitware/CMake/releases/download/v3.18.2/cmake-3.18.2.tar.gz
tar xvzf cmake-3.18.2.tar.gz
cd cmake-3.18.2
./configure
make -j8
sudo make install
cd ..
