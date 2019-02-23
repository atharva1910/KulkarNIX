#include "boot_main.h"
   
bool read_prog_headers(ELF_HEADER *elf)
{
    // Get pointer to the first program header
    ELF_PROG_HEADER *prod_head = (ELF_PROG_HEADER *)(elf + elf->e_phoff);
    if (prod_head == 0)
        return false;

    // Get pointer to the last program header
    ELF_PROG_HEADER *last_prod_head = (ELF_PROG_HEADER *)(prod_head + elf->e_phnum);
    if (last_prod_head == 0)
        return false;

    // Read each of the program header
    for(; prod_head < last_prod_head; prod_head++){
        // read each prog_header
    }

    return true;
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

ELF_HEADER *read_elf_header()
{
    ELF_HEADER *elf_head = (ELF_HEADER *)0x10000;
    uint32_t start_sector = 5;

    // Read the first sector
    read_sector(start_sector);
    ata_disk_wait();
    insw(0x1F0, (byte *)elf_head, 512/2);

    // Confirm its an elf header
    if(elf_head->ei_magic != ELF_MAGIC)
        return NULL;

    return elf_head;
}


bool read_kernel()
{
    bool bRet = false;
    ELF_HEADER *elf_head = NULL;

    if((elf_head = read_elf_header()) == NULL)
        return bRet;

    if (!read_prog_headers(elf_head))
        return bRet;
    
    bRet = true;
    return bRet;
}

void
boot_main()
{
    if(!read_kernel()){
        char *err = "Failed to read kernel";
        print_string(err);
    }
    while(1);
}
