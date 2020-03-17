#ifndef _X86_H
#define _X86_H
#include "typedefs.h"

namespace HAL{
void outb(uint16_t port, uint8_t command);
uint8_t inb(uint16_t port);
void insw(uint16_t port, BYTE *address, uint32_t count);
void EnableInterrupts();
void DisableInterrupts();
BOOL CheckIfApicExists();
}
#endif
