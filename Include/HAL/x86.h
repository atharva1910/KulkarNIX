#ifndef _X86_H
#define _X86_H
#include "typedefs.h"
void asm_outb(uint16_t port, uint8_t command);
uint8_t asm_inb(uint16_t port);
void asm_insw(uint16_t port, BYTE *address, uint32_t count);
void asm_enable_interrupts();
#endif
