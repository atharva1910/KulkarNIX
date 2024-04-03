#include <x86.h>

void HAL_outb(uint16_t port, uint8_t command)
{
  asm volatile("outb %0, %1":: "a"(command), "d"(port));
}

uint8_t HAL_inb(uint16_t port)
{
  uint8_t data = 0;
  asm volatile("inb %1, %0":"=a"(data):"d"(port));
  return data;
}

void HAL_insw(uint16_t port, byte *address, uint32_t count)
{
  asm volatile("cld; rep insw":"+D"(address), "+c"(count):"d"(port): "memory");
}


void HAL_EnableInterrupts()
{
  asm volatile("sti");
}

void HAL_DisableInterrupts()
{
  asm volatile("cli");
}

void HAL_LoadIDT(byte *idt)
{
    asm volatile("lidt %0"::"m"(*idt));

}
