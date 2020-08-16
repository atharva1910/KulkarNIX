set(CMAKE_ASM_NASM_OBJECT_FORMAT bin)
set(CMAKE_ASM_NASM_LINK_EXECUTABLE "mv <OBJECTS> ${KNIX_OUTPUT_DIR}/<TARGET>")
enable_language(ASM_NASM)

set(SRC_FILES IStageBootloader.asm)
add_executable(IStageBootloader.bin ${SRC_FILES})

#target_include_directories(IStageBootloader.bin PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")
target_compile_options(IStageBootloader.bin PRIVATE "-I ${CMAKE_CURRENT_SOURCE_DIR}/")
