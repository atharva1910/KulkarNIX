#ifndef _X86_H
#define _X86_H
#include "typedefs.h"

void asm_outb(uint16_t port, uint8_t command)
{
  asm volatile("outb %0, %1":: "a"(command), "d"(port));
}

uint8_t asm_inb(uint16_t port)
{
  uint8_t data = 0;
  asm volatile("inb %1, %0":"=a"(data):"d"(port));
  return data;
}

void asm_insw(uint16_t port, BYTE *address, uint32_t count)
{
  asm volatile("cld; rep insw":"+D"(address), "+c"(count):"d"(port): "memory");
}

void asm_load_idt(uint32_t _idt)
{
}
#endif
