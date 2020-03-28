ASMCC   = i686-elf-as
CC		= i686-elf-g++

CCFLAGS = -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -g
LDFLAGS = -ffreestanding -nostdlib 
LDFILE  = linker.ld
INCLUDES = -I.. -I../../Include
LINCLUDES = ../Lib/HAL
BUILDLOC = ../../Build

all:
	$(CC) -c Debug.cpp $(CCFLAGS) $(INCLUDES) -o $(BUILDLOC)/Debug.o

clean:
	rm -f $(BUILDLOC)/Debug.o
