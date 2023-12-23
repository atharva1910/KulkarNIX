ASMCC    = i686-elf-as
CC	 	 = i686-elf-g++
CCFLAGS  = -ffreestanding -O2 -Wall -Wextra 
LDFLAGS  = -ffreestanding -nostdlib 
LDFILE   = linker.ld
INCLUDES = -I../Common -I../Include/
LIBLOC   = ../Lib/HAL


# IDT static library
LibIDT: PIC.o x86.o Paging.o
	ar rcs $(LIBLOC)/KLibHAL.a $^ Interrupts.o 

PIC.o: 
	$(CC) -c PIC.cpp $(CCFLAGS) $(INCLUDES) -o PIC.o
	$(CC) -c Interrupts.cpp $(CCFLAGS) $(INCLUDES) -o Interrupts.o

x86.o:
	$(CC) -c x86.cpp $(CCFLAGS) $(INCLUDES) -o  x86.o

Paging.o:
	$(CC) -c Paging.cpp $(CCFLAGS) $(INCLUDES) -o Paging.o

clean:
	rm -f *.o
	rm -f *.bin
	rm -f *.img
	rm -f *.a
	rm -f *.so
