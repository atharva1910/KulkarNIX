#include "Port.h"
void
PrintString()
{
    char *buffer = (char *)0xb8000;
    char testStr[12] = "Test String";
    for(uint8_t it = 0, index = 0; it < 21; it+=2, index++){
        buffer[it + 1] = 0x07;
        buffer[it] = testStr[index];
    }
}


Port::Port(){};

void
Port::outb(uint16_t port, uint8_t command)
{
    __asm__ __volatile__("out %0, %1;"::"a"(command), "d"(port));
    return;
}

uint8_t
Port::inb(uint16_t port)
{
    uint8_t data = 0;
    __asm__ __volatile__("in %1, %0;":"=a"(data):"d"(port));
    return data;
}


void
Port::insw(uint16_t port, uint32_t address, uint16_t count)
{
    __asm__ __volatile__("cld;\
                          rep insw;"
                         :"=D"(address), "=c"(count)
                         :"d" (port)
                         : "memory");
}
