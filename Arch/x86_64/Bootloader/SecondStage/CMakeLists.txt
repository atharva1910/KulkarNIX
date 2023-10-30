SET(CMAKE_CXX_COMPILER "i686-elf-g++")
SET(SOURCES boot_main.cpp x86.cpp boot_main.s)
SET_PROPERTY(SOURCE boot_main.s SET PROPERTY LANGUAGE CXX)
ADD_EXECUTABLE(IIStageBootloader.bin ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(IIStageBootloader.bin PRIVATE ${KNIX_ARCH_INCLUDE} ${KNIX_ROOT_INC})

TARGET_COMPILE_OPTIONS(IIStageBootloader.bin PRIVATE)

TARGET_LINK_OPTIONS(IIStageBootloader.bin PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)

ADD_CUSTOM_COMMAND(TARGET IIStageBootloader.bin
 POST_BUILD
 COMMAND objcopy --only-keep-debug IIStageBootloader.bin ${KNIX_SYMBOL_DIR_PATH}/IIStageBootloader.sym)

ADD_CUSTOM_COMMAND(TARGET IIStageBootloader.bin
  POST_BUILD
  COMMAND objcopy -O binary IIStageBootloader.bin ${KNIX_OUTPUT_DIR_PATH}/IIStageBootloader.bin)
