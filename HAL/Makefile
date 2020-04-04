ASMCC    = i686-elf-as
CC	 = i686-elf-g++
CCFLAGS  = -ffreestanding -O2 -Wall -Wextra 
LDFLAGS  = -ffreestanding -nostdlib 
LDFILE   = linker.ld
INCLUDES = -I../Common -I../Include/
BUILDLOC = ../Build
LIBLOC   = ../Lib/HAL


# IDT static library
LibIDT: PIC.o x86.o Paging.o
	ar rcs $(LIBLOC)/KLibHAL.a $(BUILDLOC)/PIC.o $(BUILDLOC)/Interrupts.o $(BUILDLOC)/x86.o $(BUILDLOC)/Paging.o

PIC.o: 
	$(CC) -c PIC.cpp $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/PIC.o
	$(CC) -c Interrupts.cpp $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/Interrupts.o

x86.o:
	$(CC) -c x86.cpp $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/x86.o

Paging.o:
	$(CC) -c Paging.cpp $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/Paging.o

clean:
	rm -f $(BUILDLOC)/*.o
	rm -f $(BUILDLOC)/*.bin
	rm -f $(BUILDLOC)/*.img
	rm -f $(LIBLOC)/*.a
	rm -f $(LIBLOC)/*.so
