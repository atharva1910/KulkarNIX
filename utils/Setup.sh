# Install wget
sudo apt install wget build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo nasm

# Create working Directory
mkdir KSetup
cd KSetup

# Download gcc and wget
wget https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-10.2.0/gcc-10.2.0.tar.gz

# These are arch dependencies
# sudo apt install base-devel
# sudo apt install gmp libmpc mpfr

# UEFI
sudo apt install ovmf binutils-mingw-w64 gcc-mingw-w64 xorriso mtools

# Variables for cross compiling
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

tar -xvf binutils-2.34.tar.xz
mkdir build-binutils
cd build-binutils
../binutils-2.34/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j8
make install
cd ..

which -- $TARGET-as || echo $TARGET-as is not in the PATH
tar xvzf gcc-10.2.0.tar.gz
mkdir build-gcc
cd build-gcc
../gcc-10.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make -j8 all-gcc
make -j8 all-target-libgcc
make install-gcc
make install-target-libgcc
cd ..
