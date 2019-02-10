#ifndef _PORT_H
#define _PORT_H
#include "typedefs.h"

class Port{
 public:
    Port();
    void    outb(uint16_t port, uint8_t command);
    uint8_t inb(uint16_t port);
};
#endif
