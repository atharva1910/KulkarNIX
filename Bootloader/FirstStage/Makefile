ASMCC = nasm
ASMCCFLAGS = -f bin
OUTPUT = ../../Build/IStageBootloader.bin
INPUT = IStageBootloader.asm

all: 
	$(ASMCC) $(ASMCCFLAGS) $(INPUT) -o $(OUTPUT)

clean:
	rm -f *.o
