#ifndef _ELF_HEADER_H
#define _ELF_HEADER_H

#include "typedefs.h"
#define ELF_MAGIC 0x464c457f
#define EXE_MAX_HEADERS 3

/*
  Header taken: https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
  This is puerly a x86 header
 */
typedef struct _ELF_HEADER{
    uint32_t ei_magic;       // Magic number
    byte     ei_class;       // Class x86 or x64
    byte     ei_data;        // Endianness
    byte     ei_version;     // Version
    byte     ei_osabi;       // System V ABI 0x00
    byte     ei_abiver;      // ABI version
    byte     ei_pad[7];      // Unused
    uint16_t e_type;         // Object file id
    uint16_t e_machine;      // Instruction set
    uint32_t e_version;      // Is set to 1
    uint32_t e_entry;        // Entry point
    uint32_t e_phoff;        // Offset to program header
    uint32_t e_shoff;        // Offset to section header
    uint32_t e_flags;        // Depends upon arc type
    uint16_t e_ehsize;       // size of this header
    uint16_t e_phentsize;    // Size of program header table
    uint16_t e_phnum;        // No of entries on program header table
    uint16_t e_shentsize;    // Size of section header table
    uint16_t e_shnum;        // No of entries in section header table
    uint16_t e_shstrndx;     // Index of section header tables
}ELF_HEADER;

typedef struct ELF_PROG_HEADER{
    uint32_t p_type;         // Type of segment
    uint32_t p_offset;       // Offset of segment
    uint32_t p_vaddr;        // Virtual address of segment
    uint32_t p_paddr;        // Physical address of segment
    uint32_t p_filesz;       // Size of segment in file mem
    uint32_t p_memsz;        // Size of segment in mem
    uint32_t p_flags;        // Segment-dependant flag
    uint32_t p_align;        // Allignment
}ELF_PROG_HEADER;
#endif
