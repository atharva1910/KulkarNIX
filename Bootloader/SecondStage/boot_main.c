#include "boot_main.h"

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

/*
  Inputs:
  addr -> physical address
  offset -> offset into the file

  Outputs:
  None

  Since we know that the file _always_ resides on the KERNEL_START_SECT we can calculate the sector addres from the offset.
  Mod the offset, so we either get 0 (the offset lies on the KERNEL_START_SECT sector) or some sector number, then we read that sector.
 */
void
read_prog_header(uint32_t addr, uint32_t offset)
{
    uint32_t end_segment = addr + offset; // Points to the last address for segment
    uint32_t sect        = (offset % SECTOR_SIZE) + KERNEL_START_SECT; // Sector to read

    for(; addr < end_segment; sect++){
        read_sector(sect);
        ata_disk_wait();
        insw(0x1F0, (byte *)addr, 512/2);
        addr += SECTOR_SIZE;
    }
}

bool read_prog_headers(ELF_HEADER *elf)
{
    // Validate the number of headers
    if(elf->e_phnum > EXE_MAX_HEADERS || elf->e_phnum < 0)
        return false;
    
    // Get pointer to the first program header
    ELF_PROG_HEADER *prog_head = (ELF_PROG_HEADER *)(elf + elf->e_phoff);
    if (prog_head == 0)
        return false;

    // Get pointer to the last program header
    ELF_PROG_HEADER *last_prog_head = (ELF_PROG_HEADER *)(prog_head + elf->e_phnum);
    if (last_prog_head == 0)
        return false;

    // Read each of the program header
    for(; prog_head < last_prog_head; prog_head++){
        // read each prog_header
        read_prog_header(prog_head->p_paddr, prog_head->p_filesz);
    }
    return true;
}


ELF_HEADER *read_elf_header()
{
    ELF_HEADER *elf_head = (ELF_HEADER *)0x10000;
    dump_address(0x1234BEEF);
    asm volatile("hlt");
    
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
        char *c = "We done goofed up";
        print_string(c);
    }
    while(1);
}
