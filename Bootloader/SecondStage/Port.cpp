#include "Port.h"
Port::Port(){
};

void
Port::outb(uint16_t port, uint8_t command)
{
    __asm__ __volatile__("out %0, %1;"::"a"(command), "d"(port));
    return;
}

uint8_t
Port::inb(uint16_t port)
{
    uint8_t data;
    __asm__ __volatile__("in %1, %0;":"=a"(data):"d"(port));
    return data;
}
