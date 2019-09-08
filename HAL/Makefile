ASMCC   = i686-elf-as
CC	= i686-elf-g++

CCFLAGS = -ffreestanding -O2 -Wall -Wextra 
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld
INCLUDES = -I../Common -I../Include/
BUILDLOC = ../Build
LIBLOC   = ../Lib/HAL


# IDT static library
LibIDT: IDT.o x86.o
	ar rcs $(LIBLOC)/libIDT.a $(BUILDLOC)/IDT.o $(BUILDLOC)/Interrupts.o $(BUILDLOC)/x86.o

IDT.o: 
	$(CC) -c IDT.cpp $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/IDT.o
	$(CC) -c Interrupts.cpp $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/Interrupts.o

x86.o:
	$(CC) -c x86.cpp $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/x86.o

clean:
	rm -f $(BUILDLOC)/*.o
	rm -f $(BUILDLOC)/*.bin
	rm -f $(BUILDLOC)/*.img
	rm -f $(LIBLOC)/*.a
	rm -f $(LIBLOC)/*.so
