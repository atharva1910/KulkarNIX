#include "boot_main.h"

static inline void
print_char(char *address, char c, BYTE bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}


static inline void
print_string(char *string)
{
    BYTE bgcolor = 0x07;
    char *vga_buffer = (char *)0xb8000;
    char c = 0;
    uint32_t pos = 0;
    while((c = string[pos++]) != '\0'){
        print_char(vga_buffer, c, 0x07);
        vga_buffer += 2;
    }
}



void ata_disk_wait()
{
  while((asm_inb(0x1F7) & 0xC0) != 0x40);
}

void read_sector(uint32_t sector)
{
    ata_disk_wait(); // wait BSY to 0 and RDY to 1
    asm_outb(0x1F6, sector >> 24 | 0xE0);// Master drive
    asm_outb(0x1F2, 1); // Read one sector
    asm_outb(0x1F3, sector);
    asm_outb(0x1F4, sector >> 8);
    asm_outb(0x1F5, sector >> 16);
    // Make a read call
    asm_outb(0x1F7, 0x20);
}

/*
  Inputs:
  addr   -> physical address
  offset -> offset into the file

  Outputs:
  None

  1. Calculate the sector the section lies on
  2. Calculate the physical address to write the sector to
  3. Read sector to physical address
 */
void
read_prog_header(uint32_t addr, uint32_t filesz, uint32_t offset)
{
    uint32_t end_segment = addr + filesz; // Points to the last address for segment
    uint32_t sect        = (offset / SECTOR_SIZE) + KERNEL_START_SECT; // Sector to read
    addr -= (offset % SECTOR_SIZE); // Get to sector boundary 

    for(; addr < end_segment; sect++){
        read_sector(sect);
        ata_disk_wait();
        asm_insw(0x1F0, (BYTE *)addr, 512/2);
        addr += SECTOR_SIZE;
    }
}

ELF_HEADER *read_elf_header()
{
    ELF_HEADER *elf_head = (ELF_HEADER *)0x10000;
    
    uint32_t start_sector = 5;

    // Read the first sector
    read_sector(start_sector);
    ata_disk_wait();
    asm_insw(0x1F0, (BYTE *)elf_head, 512/2);

    // Confirm its an elf header
    if(elf_head->ei_magic != ELF_MAGIC){
        return NULL;
    }

    return elf_head;
}


uint32_t read_kernel()
{
    BOOL bRet = false;

    ELF_HEADER *elf_head = read_elf_header();

    if(elf_head  == NULL)
        return NULL;

    // Validate the number of headers
    if(elf_head->e_phnum > EXE_MAX_HEADERS || elf_head->e_phnum < 0)
        return false;

    // Get pointer to the first program header
    ELF_PROG_HEADER *prog_head = (ELF_PROG_HEADER *)((BYTE *)elf_head + elf_head->e_phoff);
    if (prog_head == 0)
        return false;

    // Read each of the program header
    for(uint32_t i = 0; i < elf_head->e_phnum; i++){
        // read each prog_header
        read_prog_header(prog_head->p_paddr, prog_head->p_filesz, prog_head->p_offset);
        // read next program header
        prog_head++;
    }
    return elf_head->e_entry;
}

extern "C"
void boot_main()
{
    void *kernel_entry = NULL;
    void (*entry)(void);

    //print_hex(0xABC);
    if((kernel_entry = (void *)read_kernel()) == NULL){
         const char *c = "Error reading Kernel :(";
         print_string((char *)c);
     }

    entry = (void (*)(void))(kernel_entry);
    entry();    // Should never return
    while(1);
}