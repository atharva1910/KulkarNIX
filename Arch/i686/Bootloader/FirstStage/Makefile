ASMCC = nasm
ASMCCFLAGS = -f bin
OUTPUT = IStageBootloader.bin
INPUT = IStageBootloader.asm

all:
	@echo "========== Building FirstStage Bootloader =========="
	$(ASMCC) $(ASMCCFLAGS) $(INPUT) -o $(OUTPUT_DIR)/$(OUTPUT)

clean:
	rm -f *.o
