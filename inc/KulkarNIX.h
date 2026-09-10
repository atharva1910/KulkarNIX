#ifndef _KULKARNIX_H
#define _KULKARNIX_H
/////////////////////////////////////////////////////////////////////////////////
// This file contains all common MACROS and defines needed for all the modules //
/////////////////////////////////////////////////////////////////////////////////

// Work around compile warnings
#define UNREFRENCED_PARAMETER(X) X;
#define KERNEL_START_PADDR 0x100000
#define KERNEL_START_VADDR 0xfffffa0000000000;
#define HIGHER_MEMORY_VADDR 0xFFFF800000000000
#endif
