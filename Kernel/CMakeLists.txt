ADD_EXECUTABLE(Kernel.bin kernel.cpp)

TARGET_COMPILE_OPTIONS(Kernel.bin PRIVATE -ffreestanding -fno-exceptions -O2 -g)

TARGET_LINK_OPTIONS(Kernel.bin PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld -ffreestanding -nostdlib)

ADD_CUSTOM_COMMAND(TARGET Kernel.bin
  POST_BUILD
  COMMAND mv Kernel.bin ${KNIX_OUTPUT_DIR})
