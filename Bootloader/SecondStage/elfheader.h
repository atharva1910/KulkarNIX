#ifndef _ELF_HEADER_H
#define _ELF_HEADER_H

#include "typedefs.h"

/*
  Header taken: https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
  This is puerly a x86 header
 */
typedef struct _ELF_HEADER{
    byte    EI_MAGIC[4];       // Magic number
    byte    EI_CLASS;          // Class x86 or x64
    byte    EI_DATA;           // Endianness
    byte    EI_VERSION;        // Version
    byte    EI_OSABI;          // System V ABI 0x00
    byte    EI_PAD[7];         // Unused
    byte    e_type[2];         // Object file id
    byte    e_machine[2];      // Instruction set
    byte    e_version[4];      // Is set to 1
    byte    e_entry[4];        // Entry point
    byte    e_phoff[4];        // Offset to program header
    byte    e_shoff[4];        // Offset to section header
    byte    e_ehsize[2];       // size of this header
    byte    e_phentsize[2];    // Size of program header table
    byte    e_phnum[2];        // No of entries on program header table
    byte    e_shentsize[2];    // Size of section header table
    byte    e_shnum[2];        // No of entries in section header table
    byte    e_shstrndx[2];     // Index of section header tables
}ELF_HEADER;
#endif
