# Install wget
sudo pacman -S wget

# Create working Directory
mkdir KSetup
cd KSetup

# Download gcc and wget
wget https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz
wget https://ftp.gnu.org/gnu/gcc/gcc-10.2.0/gcc-10.2.0.tar.gz

# Install dependencies
sudo pacman -S base-devel
sudo pacman -S gmp libmpc mpfr

# Variables for cross compiling
export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# Install binuitls
tar -xvf binutils-2.34.tar.xz
mkdir build-binutils
cd build-binutils
../binutils-2.34/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
sudo make install
cd ..

# Install gcc
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
