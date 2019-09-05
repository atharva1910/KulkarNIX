#ifndef _X86_H
#define _X86_H
#include "typedefs.h"
/*
  This header consists of the functions which are very closely coupled with x86 architecture
  The kernel will and should never use this lib directly. This should be used only by drivers for device like PIC, HDDs etc
  The Bootloader will include this directly since it is system dependant
 */
namespace x86 {
void outb(uint16_t port, uint8_t command);
uint8_t inb(uint16_t port);
void insw(uint16_t port, BYTE *address, uint32_t count);
void EnableInterrupts();
void DisableInterrupts();
} //namespace x86
#endif
