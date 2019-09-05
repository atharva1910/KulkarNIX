ASMCC   = i686-elf-as
CC	= i686-elf-g++

CCFLAGS = -ffreestanding -O2 -Wall -Wextra 
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld
INCLUDES = -I../../Common -I../../Include/
BUILDLOC = ../../Build
LIBLOC   = ../../Lib/HAL

# inline assembly lib
Libx86: x86.o
	ar rcs $(LIBLOC)/libx86.a $(BUILDLOC)/x86.o

x86.o:
	$(CC) -c x86.cpp $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/x86.o


clean:
	rm -f $(BUILDLOC)/*.o
	rm -f $(BUILDLOC)/*.bin
	rm -f $(BUILDLOC)/*.img
	rm -f $(LIBLOC)/*.a
	rm -f $(LIBLOC)/*.so
