ASMCC   = i686-elf-as
CC	= i686-elf-gcc

CCFLAGS = -ffreestanding -O2 -Wall -Wextra 
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld
INCLUDES = -I../Common -I../Include/
BUILDLOC = ./Build
LIBLOC   = ../Lib/HAL


all: LibIDT Libx86

# IDT static library
LibIDT: IDTelf32.o IDT.o
	ar rcs $(LIBLOC)/libIDT.a $(BUILDLOC)/IDTelf32.o $(BUILDLOC)/IDT.o $(BUILDLOC)/Interrupts.o 

IDTelf32.o:
	$(ASMCC) IDT.asm -o $(BUILDLOC)/IDTelf32.o

IDT.o: 
	$(CC) -c IDT.c $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/IDT.o
	$(CC) -c Interrupts.c $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/Interrupts.o

# inline assembly lib
Libx86: x86.o
	ar rcs $(LIBLOC)/libx86.a $(BUILDLOC)/x86.o

x86.o:
	$(CC) -c x86.c $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/x86.o


clean:
	rm -f $(BUILDLOC)/*.o
	rm -f $(BUILDLOC)/*.bin
	rm -f $(BUILDLOC)/*.img
	rm -f $(LIBLOC)/*.a
	rm -f $(LIBLOC)/*.so
