#include "boot_main.h"
extern void PPrintString();
   
uint32_t get_binary_size(ELF_HEADER *elf)
{
    
}

void ata_disk_wait()
{
    while((inb(0x1F7) & 0xC0) != 0x40);
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

void read_kernel(byte *address, uint32_t start_sector)
{
    // Read the first sector
    read_sector(start_sector);
    ata_disk_wait();
    insw(0x1F0, address, 512/2);

    ELF_HEADER *knix_elf_header = address;
    uint32_t bin_size = get_binary_size(knix_elf_header);
    PPrintString();
}

void
boot_main()
{
    byte *address = (byte *)0x10000;
    uint32_t start_sector = 5;
    read_kernel(address, start_sector);

    char buffer[10]  = {0};
    itoa(42, buffer);
    print_string(buffer);

    while(1);
}
