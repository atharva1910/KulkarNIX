#include "boot_main.h"
extern void ata_disk_wait();
extern void PPrintString();
    
void read_sector(uint32_t sector)
{
    // wait disk
    ata_disk_wait();
    outb(0x1F2, 1); // Read one sector
    outb(0x1F3, sector);
    outb(0x1F4, sector >> 8);
    outb(0x1F5, sector >> 16);
    outb(0x1F6, sector >> 24 | 0xE0);
    // Make a read call
    outb(0x1F7, 0x20);
    // transfere
}

void read_kernel(uint32_t address, uint32_t sector)
{
    // Perform read
    read_sector(sector);
    // wait disk
    ata_disk_wait();
    // copy to address
    insw(0x1F0, (uint32_t)address, 512/2);
}

void
boot_main()
{
    byte *address = (byte *)0x10000; // Save kernel at address
    read_kernel((uint32_t)address, 1);
}
