set(CMAKE_ASM_NASM_OBJECT_FORMAT elf32)
enable_language(ASM_NASM)

set(ASM_SRC_FILES IIStageBootloader.asm)
set(CXX_SRC_FILES boot_main.cpp x86.cpp)
add_executable(Boot.bin ${ASM_SRC_FILES} ${CXX_SRC_FILES})

target_include_directories(Boot.bin PRIVATE ${CMAKE_SOURCE_DIR}/Common ${CMAKE_SOURCE_DIR}/Include)
set_target_properties(Boot.bin PROPERTIES LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/linker.ld")
set_source_files_properties(${CXX_SRC_FILES} PROPERTIES COMPILE_FLAGS "-ffreestanding -fno-exceptions -O2")

target_link_options(Boot.bin PRIVATE -T ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld ${TEST} -ffreestanding -nostdlib -lgcc)

add_custom_command(TARGET Boot.bin
  POST_BUILD
  COMMAND mv Boot.bin ${KNIX_OUTPUT_DIR})
