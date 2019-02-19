#include "boot_main.h"
extern void PPrintString();
   
void ata_disk_wait()
{
    while(inb(0x1F7) & 0xC0 != 0x40);
}
void read_sector(uint32_t sector)
{
    ata_disk_wait(); // wait BSY to 0 and RDY to 1
    outb(0x1F6, sector >> 24 | 0xE0);// Master drive
    outb(0x1F2, 1); // Read one sector
    outb(0x1F3, sector);
    outb(0x1F4, sector >> 8);
    outb(0x1F5, sector >> 16);
    // Make a read call
    outb(0x1F7, 0x20);
}

void read_kernel(byte *address, uint32_t sector)
{
    read_sector(sector);
    ata_disk_wait();
    insw(0x1F0, address, 512/2);
    PPrintString();
}

void
boot_main()
{
    byte *address = (byte *)0x10000;
    read_kernel(address, 1);
    while(1);
}
