# --- Toolchain Configurations ---
CC := clang++
LD := ld.lld

# --- Directory & Artifact Paths ---
BUILD_DIR := ../build
TARGET    := $(BUILD_DIR)/Kernel.elf
LINKER_SCRIPT := linker.ld

# --- Source and Object Files ---
SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,$(BUILD_DIR)/kernel_%.o,$(SRCS))

# --- Compilation Flags ---
CFLAGS  := -target x86_64-unknown-none-elf \
           -ffreestanding \
           -g -O2 \
           -mno-red-zone -mno-mmx -mno-sse \
           -fno-stack-protector \
           -fno-exceptions -fno-rtti \
           -nostdlib \
           -std=c++20 \
		   -mcmodel=kernel	\
           -I../inc

LDFLAGS := -T $(LINKER_SCRIPT)

# --- Targets ---
.PHONY: all clean

all: $(TARGET)

# Link the final ELF Kernel
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $^

# Compile C++ source files into object files
$(BUILD_DIR)/kernel_%.o: %.cpp
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
