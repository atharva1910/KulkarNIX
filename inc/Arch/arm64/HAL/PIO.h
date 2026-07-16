#ifndef _PIO_H
#define _PIO_H

class PIO {
 public:
    void outb(uint16_t port, uint8_t command);
    uint8_t inb(uint16_t port);
    void insw(uint16_t port, BYTE *address, uint32_t count);
};
#endif
