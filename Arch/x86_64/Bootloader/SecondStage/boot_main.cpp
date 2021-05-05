#include "boot_main.h"
#include "Paging.h"

/*
  This file contains the main secondary boot loader and a bare bones ATA driver
  It reads the Kernel from the disk to location 0x10000 and jumps to the kernel entry point
 */

static inline void
print_char(char *address, char c, BYTE bg_color)
{
    address[1] = bg_color;
    address[0] = c;
}


static inline void
print_string(char *string)
{
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
    while((HAL::inb(0x1F7) & 0xC0) != 0x40);
}

void read_sector(uint32_t sector)
{
    ata_disk_wait(); // wait BSY to 0 and RDY to 1
    HAL::outb(0x1F6, sector >> 24 | 0xE0);// Master drive
    HAL::outb(0x1F2, 1); // Read one sector
    HAL::outb(0x1F3, sector);
    HAL::outb(0x1F4, sector >> 8);
    HAL::outb(0x1F5, sector >> 16);
    HAL::outb(0x1F7, 0x20); // Make a read call
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
        HAL::insw(0x1F0, (BYTE *)addr, 512/2);
        addr += SECTOR_SIZE;
    }
}

ELF_HEADER *read_elf_header()
{
    ELF_HEADER *elf_head = (ELF_HEADER *)KERNEL_START_PADDR;
    
    uint32_t start_sector = 5;

    // Read the first sector
    read_sector(start_sector);
    ata_disk_wait();
    HAL::insw(0x1F0, (BYTE *)elf_head, 512/2);

    // Confirm its an elf header
    if(elf_head->ei_magic != ELF_MAGIC){
        return NULL;
    }

    return elf_head;
}


uint32_t read_kernel()
{
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

void
SetupCR3PageAddress()
{
    asm volatile("movl $0x100000, %%ecx\n"
                 "movl %%ecx, %%cr3\n":::"%ecx");
}

void
SetCR4PAE()
{
    asm volatile("movl %%cr4, %%eax\n"
                 "orl $(1<<5), %%eax\n"
                 "movl %%eax, %%cr4\n":::"%eax");
}

void
SetPagingMsr()
{
    asm volatile("movl $0xC0000080, %%ecx\n"
                 "rdmsr\n"
                 "orl $(1<<8), %%eax\n"
                 "wrmsr\n":::"%eax", "%ecx");
}

void
EnablePaging()
{
    asm volatile("movl %%cr0, %%eax\n"
                 "orl $(1<<31), %%eax\n"
                 "movl %%eax, %%cr0":::"%eax");
}

void
SetupKernelPages()
{
    /* Before jumping to the Kernel we need to setup paging for long mode
      - Clear memory from 1MB to 5MB for kernel page tables
      - Create Page directory struct from 1MB to 6MB
      - Identity map first 1MB of memory
      - Map kernel to high memory (3GB-5GB)
      - Map page table above kernel 

     At this point we assume our kernel will be of 2 GB.
     To map 2GB worth of memory we need 1024 PT, 3 PDT, 1 PDPT and 1 PML4T 
     which comes around to just above 4MB so we clear out 5 MB 
     space starting from 1MB to 6MB.*/

    /* Clear Memory */
    for (uint32_t *pageAddr = (uint32_t *)KNIX_START_PAGE_ADDR;
         pageAddr < (uint32_t *)KNIX_END_PAGE_ADDR;
         pageAddr += sizeof(uint32_t)){
        *pageAddr = 0x0;
    }

    PPageMapLevel4Table pml4t = (PPageMapLevel4Table )KNIX_START_PAGE_ADDR;

    /* Allocate space for one PML4T */
    PPageDirPtrTable pdpt     = (PPageDirPtrTable )(pml4t + 1);

    /* Allocate space for one PDPT */
    PPageDirTable pdt         = (PPageDirTable )(pdpt + 1);
 
    /* Allocate space for three  PDT */
    PPageTable pt             = (PPageTable )(pdt + 3);

    pml4t->pageMapLevel4Entry[0].ui64pml4Entry = (uint64_t)pdpt| 0x3;
    pdpt->pageDirPtrEntry[0].ui64pdpEntry    = (uint64_t)pdt | 0x3;
    pdt->pageDirEntry[0].ui64pdEntry       = (uint64_t)pt  | 0x3;

#if 1
    /* Identity map the first MB */
    uint64_t vAddress = 0x00 | 0x3;
    for(uint32_t pteIdx = 0; pteIdx < 256; pteIdx++){
        pt->pageTableEntry[pteIdx].ui64ptEntry = vAddress;
        vAddress  += 0x1000;
    }

    /* The kernel will be mapped from 3GB to 5GB */
    vAddress = KERNEL_START_PADDR | 0x3;

    pdt++; 
    pdpt->pageDirPtrEntry[0x3].ui64pdpEntry = (uint64_t)pdt | 0x3;

    /* Map the 1 GB to 3GB-4GB address */
    for (uint32_t pdtIdx = 0; pdtIdx < 512; pdtIdx++) {
        pt++;
        pdt->pageDirEntry[pdtIdx].ui64pdpEntry = (uint64_t)pt | 0x3;
        for (uint32_t ptIdx = 0; ptIdx < 512; ptIdx++) {
            pt->pageTableEntry[ptIdx].ui64ptEntry = vAddress;
            vAddress += 0x1000;
        }
    }

    pdt++;
    pdpt->pageDirPtrEntry[0x4].ui64pdpEntry = (uint64_t)pdt | 0x3;

    /* Map the 1 GB to 4GB-5GB address 
       Since the kernel is mapped starting from 6MB
       We will over shoot the 2GB mark if we map the entire thing
       So we dont map the last 4 MB */
    for (uint32_t pdtIdx = 0; pdtIdx < 510; pdtIdx++) {
        pt++;
        pdt->pageDirEntry[pdtIdx].ui64pdpEntry = (uint64_t)pt | 0x3;
        for (uint32_t ptIdx = 0; ptIdx < 512; ptIdx++) {
            pt->pageTableEntry[ptIdx].ui64ptEntry = vAddress;
            vAddress += 0x1000;
        }
    }

    /* We need to map the page tables to in the above hole of 4 MB 
       Its not really 4 MB. We need 5MB worth of tables to map kernel tables into itself
       So we use the remaining 4 MB and one MB from the mapped space.
       The only problem is this address is strange to remember will do this later */
#else
    /* Identity mapping for testing */
    uint64_t vAddress = 0x00 | 0x3;

    for (uint32_t pdptIdx = 0; pdptIdx < 3; pdptIdx++){
        for (uint32_t pdtIdx = 0; pdtIdx < 512; pdtIdx++){
            for(uint32_t pteIdx = 0; pteIdx < 512; pteIdx++){
                pt->pte[pteIdx].ui64ptEntry = vAddress;
                vAddress  += 0x1000;
            }
            pdt->pde[pdtIdx].ui64pdEntry = (uint64_t)pt | 0x3;
            pt++;
        }
        pdpt->pdpe[pdptIdx].ui64pdpEntry = (uint64_t)pdt | 0x3;
        pdt++; 
    }

#endif
}

void
SetupLongModeKernelPaging()
{
    SetupKernelPages();
    SetupCR3PageAddress();
    SetCR4PAE();
    SetPagingMsr();
    EnablePaging();
    print_string("OK ALL DONE");
    asm volatile("hlt");
}

extern "C"
void boot_main()
{
    void *kernel_entry = NULL;
    const char *c = NULL;
    void (*entry)(void);

    if((kernel_entry = (void *)read_kernel()) == NULL){
         c = "Error reading Kernel :(";
         print_string((char *)c);
         asm volatile("hlt");
    }

    SetupLongModeKernelPaging();

    /* We are now ok to jump to kernel */
    entry = (void (*)(void))(kernel_entry);
    entry();    // Should never return

    c = "Kernel Panic! Returned from entry()";
    print_string((char *)c);
}

__asm__(
    ".section .text\n"
    ".global __start\n"
    ".type   __start, @function\n\n"
"__start:\n"
    "cli\n"
    "pop     %ax\n"
    "mov     %ax, %ds\n"
    "mov     %ax, %ss\n"
    "mov     %ax, %es\n"
    "mov     $0x07c00, %sp\n"
    "call    boot_main\n"
    "hlt\n"
);
