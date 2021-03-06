void
PIO::outb(uint16_t port, uint8_t command)
{
  asm volatile("outb %0, %1":: "a"(command), "d"(port));
}

uint8_t
PIO::inb(uint16_t port)
{
  uint8_t data = 0;
  asm volatile("inb %1, %0":"=a"(data):"d"(port));
  return data;
}

void
PIO::insw(uint16_t port, BYTE *address, uint32_t count)
{
  asm volatile("cld; rep insw":"+D"(address), "+c"(count):"d"(port): "memory");
}

