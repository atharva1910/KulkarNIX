#include "HAL/x86.h"
//#include "x86lib.h"

namespace x86 {
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

void insw(uint16_t port, BYTE *address, uint32_t count)
{
  asm volatile("cld; rep insw":"+D"(address), "+c"(count):"d"(port): "memory");
}


void EnableInterrupts()
{
  asm volatile("sti");
}

void DisableInterrupts()
{
  asm volatile("cli");
}

} // namespace x86
