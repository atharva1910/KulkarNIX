#ifndef _X86_H
#define _X86_H
#include "typedefs.h"

void outb(uint16_t port, uint8_t command)
{
    asm volatile("outb %0, %1":: "a"(command), "d"(port));
}

uint8_t inb(uint16_t port)
{
    uint8_t data = 0;
    asm volatile("inb %1, %0":"=a"(data):"d"(port));
    return data;
}
#endif
