#include "boot_main.h"
extern void ata_bsy_wait();
extern void ata_drq_wait();

void read_sector(uint32_t address)
{
    // wait till hdd ready
    ata_bsy_wait();
    // make read operation
    outb(0x1F2, 1); // Read one sector
    outb(0x1F3, address & 0xFF);
    outb(0x1F4, (address >> 8) & 0xFF);
    outb(0x1F5, (address >> 16) & 0xFF);
    // Make a read call
    outb(0x1F7, 20);
    // Wait till hdd ready to transfer
    ata_drq_wait();
    // transfer
}

void read_kernel(byte *address)
{

}

void
boot_main()
{
    byte *address = (byte *)0x10000; // Save kernel at address
    read_kernel(*(uint32_t *)address);
    while(1);
}
