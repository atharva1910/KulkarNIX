ADD_EXECUTABLE(IIStageBootloader.bin boot_main.cpp x86.cpp)

TARGET_INCLUDE_DIRECTORIES(IIStageBootloader.bin PRIVATE ${KNIX_ARCH_INCLUDE} ${KNIX_ROOT_INC})

# Setup the correct compiler
SET(CMAKE_CXX_COMPILER "i686-elf-g++")

TARGET_COMPILE_OPTIONS(IIStageBootloader.bin PRIVATE)

TARGET_LINK_OPTIONS(IIStageBootloader.bin PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld)

ADD_CUSTOM_COMMAND(TARGET IIStageBootloader.bin
 POST_BUILD
 COMMAND objcopy --only-keep-debug IIStageBootloader.bin ${KNIX_OUTPUT_DIR_PATH}/IIStageBootloader.sym)

ADD_CUSTOM_COMMAND(TARGET IIStageBootloader.bin
 POST_BUILD
 COMMAND objcopy -O binary IIStageBootloader.bin ${KNIX_OUTPUT_DIR_PATH}/IIStageBootloader.bin)
