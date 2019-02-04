ASMCC = nasm
ASMCCFLAGS = -f bin
OUTPUT = ../../Build/IStageBootloader.bin

all: Bootloader.asm
	$(ASMCC) $(ASMCCFLAGS) $^ -o $(OUTPUT)

clean:
	rm -f *.o
